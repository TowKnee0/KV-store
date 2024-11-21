#include "ext_hash.h"
#include <cmath>
#include <iostream>
#include <algorithm>
#include <stdexcept>
using namespace std;


/**
 * @brief Constructs an ExtHash object.
 *
 * @param minimumSizeInPages The minimum number of pages the hash table can have.
 * @param maximumSizeInPages The maximum number of pages the hash table can have.
 * @param expand_factor The factor by which the hash table expands.
 * @param frame_size The size of each frame in pages.
 */
ExtHash::ExtHash(int minimumSizeInPages, int maximumSizeInPages, float expand_factor, int frame_size, shared_ptr<IO> io_handler)
    : minimumSizeInPages(minimumSizeInPages), maximumSizeInPages(maximumSizeInPages), currentSizeInPages(minimumSizeInPages), expand_factor(expand_factor), frame_size(frame_size), item_count(0), overflow_count(0), io_handler(io_handler)
{

    // Each frame can store a maximum of frame_size pages.
    directory_bit_size = bitSizeForNumberOfPages(minimumSizeInPages, frame_size);

    // Resize the directory.
    // Initialize the directory with null pointers.
    this->directory.resize(pow(2, directory_bit_size));
    this->bucket_counts.resize(pow(2, directory_bit_size), 0);

    this->clock.resize(maximumSizeInPages);
    this->curr_hand = 0;

}

/**
 * @brief Computes the hash of the filename and page number.
 *
 * @param filename The name of the file.
 * @param pageNumber The page number associated with the file.
 * @return xxh::hash_t<32> The computed 32-bit hash value.
 */
xxh::hash_t<32> ExtHash::getHash(const string filename, int pageNumber)
{
    return xxh::xxhash<32>(filename + to_string(pageNumber));
}

uint32_t ExtHash::GetIndexFromHashAndBitSize(xxh::hash_t<32> hash, int bit_size)
{
    return hash & ((1 << bit_size) - 1);
}
/**
 * @brief Creates a new frame.
 *
 * @param filename The name of the file associated with the frame. Default is an empty string.
 * @param pageNumber The page number associated with the frame. Default is -1.
 * @param data The data to be stored in the frame as a vector of pairs of integers. Default is an empty vector.
 * @param dirty A boolean flag indicating whether the frame is dirty. Default is false.
 * @return shared_ptr<frame> A shared pointer to the newly created frame.
 *
 * The function initializes a new frame.
 * If all parameters are default values, the frame is marked as not in use.
 */
shared_ptr<frame> ExtHash::createFrame(const string filename, int pageNumber, vector<pair<int, int>> data, bool dirty = false)
{
    shared_ptr<frame> new_frame = make_shared<frame>();
    new_frame->filename = filename;
    new_frame->pageNumber = pageNumber;
    new_frame->data = data;
    new_frame->hash = this->getHash(filename, pageNumber);
    new_frame->dirty = false;
    new_frame->next = nullptr;
    new_frame->recently_used = true;

    return new_frame;
}

/**
 * @brief Calculates the bit size required for a given number of pages and a frame size.
 *
 * @param numberOfPages The total number of pages.
 * @param frame_size The size of each frame.
 * @return The bit size required to represent the number of pages.
 */
int ExtHash::bitSizeForNumberOfPages(int numberOfPages, int frame_size)
{
    if (numberOfPages == 0)
    {
        return 1;
    }
    int bit_size = ceil(log2(ceil(static_cast<double>(numberOfPages) / frame_size)));
    // Edge case, we always need 1 bit.
    if (bit_size == 0)
    {
        return 1;
    }
    return bit_size;
}

/**
 * @brief Updates the maximum size of the hash table in pages.
 *
 * This function sets a new maximum size for the hash table. If the new maximum size
 * is smaller than the current maximum size, the directory is shrunk to fit the new size.
 *
 * @param newMaximumSizeInPages The new maximum size in pages.
 */
void ExtHash::updateMaximumSizeInPages(int newMaximumSizeInPages)
{

    if (newMaximumSizeInPages < this->maximumSizeInPages)
    {
        this->shrinkDirectory(newMaximumSizeInPages);
    }
    this->maximumSizeInPages = newMaximumSizeInPages;
}



/**
 * @brief Inserts a NEW page into the hash table.
 *
 * May evict several pages to make room for the new page.
 *
 * @param filename The name of the file associated with the page.
 * @param pageNumber The page number within the file.
 * @param data The data to be stored in the page, represented as a vector of key-value pairs.
 * @param dirty A boolean flag indicating whether the page is dirty. Default is false.
 */
void ExtHash::putPage(const string filename, int pageNumber, vector<pair<int, int>> data, bool dirty)
{

    // Before inserting a new page, check if we are full.
    // If so, evict a page before inserting this one.
    if (this->item_count >= this->maximumSizeInPages)
    {
        this->evict();
    }

    uint32_t hash = this->getHash(filename, pageNumber);
    uint32_t index = this->GetIndexFromHashAndBitSize(hash, this->directory_bit_size);
    
    shared_ptr<frame> new_frame = createFrame(filename, pageNumber, data, dirty);

    // bucket is empty, insert the frame
    if (this->directory[index] == nullptr)
    {
        this->directory[index] = new_frame;
    } else {
        // bucket is not empty, insert the frame into the second position.
        // IMPORTANT: WE DO THIS BECAUSE OTHERWISE IF MULTIPLE INDEXES POINT TO THE SAME BUCKET,
        // ONLY ONE WOULD BE UPDATED TO HAVE THE NEW HEAD.
        new_frame->next = this->directory[index]->next;
        this->directory[index]->next = new_frame;
    }

    this->clock[this->curr_hand] = new_frame;
    this->curr_hand = (this->curr_hand + 1) % clock.size();

    this->bucket_counts[index]++;
    this->item_count++;
    // Check if we are at a new threshold to expand.
    this->checkAndExpand();
    // Check if we need to split the bucket
    this->checkAndSplitBucket(index);

}

/**
 * @brief Checks if the given bucket can be split, if so it attempts to split the bucket.
 * 
 * Note that this may still rehash all the frames to the same bucket.
 * 
 * @param index The index of the bucket to check and potentially split.
 */
void ExtHash::checkAndSplitBucket(int index)
{
    // No need to split
    if (this->bucket_counts[index] <= this->frame_size){
        return;
    }

    shared_ptr<frame> old_bucket = this->directory[index];

    // Loop over all the indicies in the directory and find the indicies which point to the old frame.
    // We then update those indicies to be nullpointer so we can reinsert the frames.
    int count = 0;

    int prev_bucket_count = this->bucket_counts[index];
    for (int i = 0; i < this->directory.size(); i++)
    {
        if (this->directory[i] == old_bucket)
        {
            this->directory[i] = nullptr;
            this->bucket_counts[i] = 0;
            count++;
        }
    }

    // Nothing to split to, keep the old bucket the same.
    if (count == 1){
        this->directory[index] = old_bucket;
        this->bucket_counts[index] = prev_bucket_count;
        return;
    }
    // Now we need to reinsert all the frames in the old bucket into the directory.
    shared_ptr<frame> next_frame;
    while (old_bucket != nullptr)
    {
        next_frame = old_bucket->next;
        old_bucket->next = nullptr;
        

        // Reinsert
        uint32_t new_index = this->GetIndexFromHashAndBitSize(old_bucket->hash, this->directory_bit_size);
        if (this->directory[new_index] == nullptr){
            this->directory[new_index] = old_bucket;
        } else {
            // bucket is not empty, insert the frame into the second position.
            // IMPORTANT: WE DO THIS BECAUSE OTHERWISE IF MULTIPLE INDEXES POINT TO THE SAME BUCKET,
            // ONLY ONE WOULD BE UPDATED TO HAVE THE NEW HEAD.
            old_bucket->next = this->directory[new_index]->next;
            this->directory[new_index]->next = old_bucket;
        }
        this->bucket_counts[new_index]++;
        old_bucket = next_frame;
    }
}


/**
 * @brief Checks if we have enough items in the table to warrant an expansion.
 * If we do, we expand the directory.
 */
void ExtHash::checkAndExpand()
{

    double max_items_in_table = this->directory.size() * this->frame_size;
    // Example 10 >= 100 * .8 = 80 => False
    if (this->item_count >= this->expand_factor * max_items_in_table){
        this->directory_bit_size++;

        int prev_size = this->directory.size();
        for (int i = 0; i < prev_size; i++)
        {
            this->directory.push_back(this->directory[i]);
            this->bucket_counts.push_back(this->bucket_counts[i]);
        }
    }
}

bool rangeIsNullpointer(vector<shared_ptr<frame>> &vec, int start, int end){
    for (int i = start; i <= end; i++){
        if (vec[i] != nullptr){
            return false;
        }
    }
    return true;
}

/**
 * @brief Shrinks the directory to the specified number of pages.
 *
 * It calculates the new directory bit size and the number of indices based on the target number of pages.
 * If the current number of items exceeds the new possible total, it evicts random items until the count is within the limit.
 * It then updates the directory bit size and maximum size in pages.
 * For each old index that is no longer valid, it reinserts the frames using the new directory bit size.
 * Finally, it resizes the directory to the new size.
 *
 * @param targetNumberOfPages The target number of pages to shrink the directory to.
 */
void ExtHash::shrinkDirectory(int targetNumberOfPages)
{
    if (targetNumberOfPages < this->minimumSizeInPages)
    {
        throw runtime_error("Cannot shrink below the minimum size.");
    } else if (targetNumberOfPages > this->item_count){
        // If we want to shrink the max to a value larger than the current item count, we can just return.
        return;
    }
    
    int bits_for_shrink = bitSizeForNumberOfPages(targetNumberOfPages, this->frame_size);
    
    // If we need the same amount of bits, we can just evict until we reach the target.
    if (bits_for_shrink == this->directory_bit_size){
        while (this->item_count > targetNumberOfPages){
            this->evict();
        }
    } else {
        // Find the index range that we need to evict.
        int last_index = directory.size() - 1;
        int first_index = pow(2, bits_for_shrink);

        while (!rangeIsNullpointer(this->directory, first_index, last_index)){
            this->evict();
        }

        // Resize all the vectors to the correct size
        this->directory.resize(pow(2, bits_for_shrink));
        this->bucket_counts.resize(pow(2, bits_for_shrink), 0);
        this->clock.resize(targetNumberOfPages);
        this->directory_bit_size = bits_for_shrink;
        // Clock hand needs to be at a null pointer.
        // Not gauranteed to be at a null pointer after resizing.
        // Just evict whatever is first in the clock.
        this->curr_hand = 0;
        if (this->clock[0] == nullptr){
            return;
        }
        this->clock[0]->recently_used = false;
        this->evict();

    }
}

/**
 * @brief Retrieves the page data associated with a given filename and page number.
 *
 * If the page is not in the hash table read it from storage.
 *
 * @param filename The name of the file for which the page data is being retrieved.
 * @param pageNumber The page number within the file.
 * @return An optional containing a vector of pairs of integers representing the page data,
 *         or an empty optional if no matching frame is found.
 */
optional<vector<pair<int, int>>> ExtHash::getPage(const string filename, int pageNumber)
{
    // Calculate the hash of the filename and pageNumber
    xxh::hash_t<32> hash = this->getHash(filename, pageNumber);

    // Get which index this hash should be stored in.
    uint32_t index = this->GetIndexFromHashAndBitSize(hash, this->directory_bit_size);
    shared_ptr<frame> current_frame = this->directory[index];
    while (current_frame != nullptr)
    {
        if (current_frame->pageNumber == pageNumber && current_frame->filename == filename)
        {
            current_frame->recently_used = true;
            return current_frame->data;
        }
        current_frame = current_frame->next;
    }

    // If we reach here, the page is not in the hash table. Try and read it from the file.
    optional<vector<pair<int, int>>> page = this->io_handler->readPageFromFile(filename, pageNumber);
    if (page.has_value())
    {
        this->putPage(filename, pageNumber, page.value());

        return page;
    }

    return nullopt;
}

void ExtHash::evict()
{
    bool found_not_recently_used = false;
    int loop_count = 0;
    while (!found_not_recently_used){
        // If we have looped through 2 times without anything to evict (ie everything is null)
        // then we are done.
        if (loop_count > 2 * this->clock.size() && this->clock[this->curr_hand] == nullptr){
            return;
        }
        if (this->clock[this->curr_hand] == nullptr){
            this->curr_hand = (this->curr_hand + 1) % clock.size();
            loop_count++;
            continue;
        }

        if (this->clock[this->curr_hand]->recently_used == false){
            found_not_recently_used = true;
        } else {
            this->clock[this->curr_hand]->recently_used = false;
            this->curr_hand = (this->curr_hand + 1) % clock.size();
        }
    }


    shared_ptr<frame> evicting = this->clock[this->curr_hand];
    if (evicting == nullptr)
    {
        return;
    }

    int idx = GetIndexFromHashAndBitSize(evicting->hash, this->directory_bit_size);
    if (this->directory[idx] == evicting)
    {

        // If we are evicting the first element in the bucket, we need to loop over the whole directory
        // since it may be the head of multiple entries in the directory.
        for (int i = 0; i < this->directory.size(); i++)
        {
            if (this->directory[i] == evicting)
            {
                this->bucket_counts[i]--;
                this->directory[i] = evicting->next;
            }
        }

        this->directory[idx] = evicting->next;
    }
    else
    {
        shared_ptr<frame> prev = this->directory[idx];
        while (prev != nullptr && prev->next != evicting)
        {
            prev = prev->next;
        }

        if (prev == nullptr)
        {
            throw runtime_error("data loss: page in clock but not directory");
        }
        prev->next = prev->next->next;
        this->bucket_counts[idx]--;
    }
    this->item_count--;
    this->clock[this->curr_hand] = nullptr;
    if (evicting->dirty)
    {
        this->io_handler->writeToFile(evicting->filename, evicting->data, evicting->pageNumber);
    }
}