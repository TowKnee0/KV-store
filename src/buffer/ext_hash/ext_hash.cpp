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
 * @brief Checks if the given overflow value exceeds the frame size and handles the overflow if necessary.
 *
 * This function first checks if the overflow value is greater than the frame size. If it is, it attempts to handle
 * the overflow by calling the HandleOverflow function. If the overflow is successfully handled, the function returns.
 * Otherwise, it enters a loop where it evicts random entries until the index is no longer overflowing.
 *
 * @param overflow_value The value indicating the current overflow status.
 * @param index The index at which the overflow is being checked and handled.
 */
void ExtHash::checkAndHandleOverflow(int overflow_value, int index)
{
    cout << "checkAndHandleOverflow 1" << endl;
    if (overflow_value > this->frame_size)
    {
        bool overflow_handled = this->HandleOverflow(index);
        if (overflow_handled)
        {
            return;
        }
    }
}

/**
 * @brief Checks if the index in the hash table is overflowing.
 *
 * Counts the number of frames in use for this index.
 *
 * @param index The index in the hash table to check for overflow.
 * @return true if the index is overflowing, false otherwise.
 */
bool ExtHash::IndexIsOverflowing(int index)
{
    int frame_count = 0;
    shared_ptr<frame> current_frame = this->directory[index];

    while (current_frame->next != nullptr)
    {
        current_frame = current_frame->next;
        frame_count++;
    }

    return frame_count > this->frame_size;
}

/**
 * @brief Handles the overflow of a hash table frame by splitting. If we split and
 * still have an overflow, try expanding the directory and splitting again.
 * If we cannot split and expand, return false to indicate that eviction is required.
 *
 * @param overflowing_index The index of the bucket that has overflowed.
 * @return true if the overflow was successfully handled and entries were redistributed.
 * @return false if the overflow could not be handled and eviction is required.
 */
bool ExtHash::HandleOverflow(uint32_t overflowing_index)
{
    cout << "HandleOverflow 1" << endl;
    shared_ptr<frame> old_frame = this->directory[overflowing_index];

    // print the old frame
    shared_ptr<frame> tmp = this->directory[overflowing_index];
    while (tmp != nullptr)
    {
        cout << "pointer address " << tmp << endl;
        cout << "old_frame filename is " << tmp->filename << endl;
        cout << "old_frame pageNumber is " << tmp->pageNumber << endl;
        tmp = tmp->next;
    }

    // Loop over all the indicies in the directory and find the indicies which point to the old frame.
    // We then update those indicies to point at a new empty frame.
    int count = 0;
    for (int i = 0; i < this->directory.size(); i++)
    {
        if (this->directory[i] == old_frame)
        {
            this->directory[i] = nullptr;
            count++;
        }
    }

    // If we cannot split the current index, return false and let the eviction handle it.
    if (count < 2)
    {
        directory[overflowing_index] = old_frame;
        return false;
    }
    // Now we need to reinsert all the frames in the old frame into the directory.
    shared_ptr<frame> current_frame = old_frame;

    // Insert without using putPage to avoid infinite recursion (If we reinset all into the same index (bad luck)).
    bool all_same_index = true;
    uint32_t first_hash = current_frame->hash;
    uint32_t prev_index = this->GetIndexFromHashAndBitSize(first_hash, this->directory_bit_size);
    cout << "HandleOverflow 1.5" << endl;
    while (current_frame != nullptr)
    {
        xxh::hash_t<32> hash = current_frame->hash;

        // Get which index this hash should be stored in by selecting the first directory_bit_size bits of the hash.
        uint32_t index = this->GetIndexFromHashAndBitSize(hash, this->directory_bit_size);

        if (index != prev_index)
        {
            all_same_index = false;
        }

        // Insert without checking for overflow, we will handle it after.
        // Either we insert it into the same index, or we insert it into a new index.
        cout << "HandleOverflow 2, trying to insert into " << index << endl;
        insertFrame(current_frame, index, false);
        cout << "HandleOverflow 3" << endl;

        shared_ptr<frame> next_frame = current_frame->next;
        if (next_frame != nullptr)
        {
            cout << "next frame address is " << next_frame << endl;
            cout << "next_frame filename is " << next_frame->filename << endl;
            cout << "next_frame pageNumber is " << next_frame->pageNumber << endl;
        }
        current_frame->next = nullptr;
        current_frame = next_frame;
    }
    cout << "HandleOverflow 4" << endl;
    if (all_same_index && this->canExpand())
    {
        cout << "HandleOverflow 5" << endl;

        this->expandDirectory();
        bool handled = this->HandleOverflow(prev_index);
        if (not handled)
        {
            return false;
        }
    }
    else if (all_same_index)
    {
        cout << "HandleOverflow 6" << endl;

        return false;
    }
    return true;
};

/**
 * @brief Inserts a new frame into the hash table at the specified index.
 *
 * @param new_frame A shared pointer to the frame to be inserted.
 * @param index The index in the hash table where the frame should be inserted.
 * @param check_overflow A boolean flag indicating whether to check and handle
 * overflow conditions.
 */
void ExtHash::insertFrame(shared_ptr<frame> new_frame, uint32_t index, bool check_overflow)
{   
    cerr << "inserting frame 1 with index " << index <<  endl;
    cout << "inserting frame 1 with index " << index <<  endl;

    if (directory[index] == nullptr)
    {
        cout << "inserting frame 2" << endl;
        
        directory[index] = new_frame;
    }
    else
    {
        cout << "inserting frame 3" << endl;

        int overflow_count = 1;
        shared_ptr<frame> current_frame = directory[index];
        while (current_frame->next != nullptr)
        {
            overflow_count++;
            current_frame = current_frame->next;
        }
        cout << "inserting frame 4" << endl;

        current_frame->next = new_frame;
        overflow_count++;
        if (check_overflow)
        {
            cout << "inserting frame 5" << endl;

            this->checkAndHandleOverflow(overflow_count, index);
        }
        cout << "inserting frame 6" << endl;

    }
    cout << "inserting frame 7" << endl;
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
        // TODO: resize here after evicting
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
    

    // bucket is empty, insert the frame
    if (this->directory[index] == nullptr)
    {
        shared_ptr<frame> new_frame = createFrame(filename, pageNumber, data, dirty);
        insertFrame(new_frame, index, true);
        return;
    } else {
        // bucket is not empty, insert the frame into the second position.
        // IMPORTANT: WE DO THIS BECAUSE OTHERWISE IF MULTIPLE INDEXES POINT TO THE SAME BUCKET,
        // ONLY ONE WOULD BE UPDATED TO HAVE THE NEW HEAD.
        shared_ptr<frame> new_frame = createFrame(filename, pageNumber, data, dirty);
        new_frame->next = this->directory[index]->next;
        this->directory[index]->next = new_frame;
    }

    this->bucket_counts[index]++;
    this->item_count++;
    // Check if we are at a new threshold to expand.
    this->checkAndExpand();
    // Check if we need to split the bucket
    this->checkAndSplitBucket(index);

    // // If its already in the table, update the data and return.
    // optional<vector<pair<int, int>>> page = this->getPage(filename, pageNumber, false);
    // cout << "putting page into the hash table" << endl;
    // if (page.has_value())
    // {
    //     // Nothing to be done
    //     if (page.value() == data)
    //     {
    //         return;
    //     }

    //     uint32_t index = this->GetIndexFromHashAndBitSize(this->getHash(filename, pageNumber), this->directory_bit_size);
    //     shared_ptr<frame> current_frame = this->directory[index];
    //     while (current_frame != nullptr)
    //     {
    //         if (current_frame->filename == filename && current_frame->pageNumber == pageNumber)
    //         {
    //             current_frame->data = data;
    //             current_frame->dirty = true;
    //             current_frame->recently_used = true;
    //             return;
    //         }
    //         current_frame = current_frame->next;
    //     }
    //     return;
    // }
    // cout << "putting page 1 " << endl;
    // shared_ptr<frame> new_frame = createFrame(filename, pageNumber, data, dirty);
    // cout << "putting page 2 " << endl;

    // uint32_t index = GetIndexFromHashAndBitSize(new_frame->hash, this->directory_bit_size);

    // this->evict();
    // cout << "putting page 3 " << endl;

    // insertFrame(new_frame, index, true);
    // cout << "putting page 4 " << endl;

    // this->clock[this->curr_hand] = new_frame;
    // this->curr_hand = (this->curr_hand + 1) % clock.size();
    // cout << "putting page 5 " << endl;

    // this->item_count++;
    // this->checkAndExpand();
    // cout << "putting page 6 " << endl;

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
    for (int i = 0; i < this->directory.size(); i++)
    {
        if (this->directory[i] == old_bucket)
        {
            this->directory[i] = nullptr;
            count++;
        }
    }

    // Nothing to split to, keep the old bucket the same.
    if (count == 1){
        this->directory[index] = old_bucket;
        return;
    }
    
    // Now we need to reinsert all the frames in the old bucket into the directory.
    while (old_bucket != nullptr)
    {
        shared_ptr<frame> next_frame = old_bucket->next;
        old_bucket->next = nullptr;
        

        // Reinsert
        uint32_t new_index = this->GetIndexFromHashAndBitSize(old_bucket->hash, this->directory_bit_size);
        if (this->directory[new_index] == nullptr){
            this->directory[new_index] = old_bucket;
        } else {
            // bucket is not empty, insert the frame into the second position.
            // IMPORTANT: WE DO THIS BECAUSE OTHERWISE IF MULTIPLE INDEXES POINT TO THE SAME BUCKET,
            // ONLY ONE WOULD BE UPDATED TO HAVE THE NEW HEAD.
            old_bucket->next = this->directory[index]->next;
            this->directory[index]->next = old_bucket;
        }
        old_bucket = next_frame;
    }

}

/**
 * @brief Determines if the hash table can expand.
 *
 * @return true if the hash table can expand, false otherwise.
 */
bool ExtHash::canExpand()
{
    return this->currentSizeInPages * 2 <= this->maximumSizeInPages;
}

/**
 * @brief Checks if we have enough items in the table to warrant an expansion.
 * If we do, we expand the directory.
 */
void ExtHash::checkAndExpand()
{
    
    double factor = (this->currentSizeInPages * this->expand_factor);
    if (this->item_count >= factor && this->canExpand())
    {
        this->expandDirectory();
    }
}

/**
 * @brief Expands the directory by doubling its size.
 * Sets the new directory pointers to the old directory pointers.
 */
void ExtHash::expandDirectory()
{
    // Double the size of the directory.
    this->directory_bit_size++;
    this->currentSizeInPages *= 2;

    // Each frame the the directory gets doubled (ie 10 and 00 need to point at the same frame)
    int prev_size = this->directory.size();
    for (int i = 0; i < prev_size; i++)
    {
        this->directory.push_back(this->directory[i]);
    }
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
    int target_directory_bit_size = this->bitSizeForNumberOfPages(targetNumberOfPages, this->frame_size);
    int target_number_of_indices = pow(2, target_directory_bit_size);
    int current_number_of_indices = pow(2, this->directory_bit_size);

    int new_posssible_total = target_number_of_indices * this->frame_size;
    int evict_num = this->item_count - new_posssible_total;
    for (int i = 0; i < evict_num; i++)
    {
        this->evict();
        this->curr_hand = (this->curr_hand + 1) % clock.size();
    }
    clock.erase(remove_if(clock.begin(), clock.end(), [](shared_ptr<frame> p)
                               { return p == nullptr; }),
                clock.end());

    this->directory_bit_size = target_directory_bit_size;
    this->maximumSizeInPages = targetNumberOfPages;

    // For each of old indices between target_number_of_indices and current_number_of_indices, we need to reinsert them
    // only using the first target_directory_bit_size bits of the hash.

    for (int i = target_number_of_indices; i < current_number_of_indices; i++)
    {
        shared_ptr<frame> current_frame = this->directory[i];
        while (current_frame != nullptr)
        {
            this->item_count--; // Putting the frame back in will increase the item count, this offsets it.
            insertFrame(current_frame, GetIndexFromHashAndBitSize(current_frame->hash, target_directory_bit_size), false);
            shared_ptr<frame> next_frame = current_frame->next;
            current_frame->next = nullptr;
            current_frame = next_frame;
        }
    }

    // Resize the directory to the new size.
    this->directory.resize(pow(2, this->directory_bit_size));
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
optional<vector<pair<int, int>>> ExtHash::getPage(const string filename, int pageNumber, bool read_from_storage)
{
    cout << "read from storage flag is " << read_from_storage << endl;
    // Calculate the hash of the filename and pageNumber
    xxh::hash_t<32> hash = this->getHash(filename, pageNumber);

    // Get which index this hash should be stored in.

    // Select the first directory_bit_size bits of the hash.
    uint32_t index = this->GetIndexFromHashAndBitSize(hash, this->directory_bit_size);
    cout << "index " << index << endl;
    shared_ptr<frame> current_frame = this->directory[index];
    while (current_frame != nullptr)
    {
        if (current_frame->pageNumber == pageNumber && current_frame->filename == filename)
        {
            current_frame->recently_used = true;
            cout << "returning data" << endl;
            return current_frame->data;
        }
        current_frame = current_frame->next;
        cout << "in while" << endl;
    }

    // TODO: DELETE
    // shared_ptr<frame> tmp;
    // for (int idx = 0; idx < this->directory.size(); idx++ ){
    //     if (this->directory[idx] == nullptr) {
    //         continue;
    //     }

    //     tmp = this->directory[idx];
    //     while (tmp != nullptr) {
    //         if (tmp->filename == filename && tmp->pageNumber == pageNumber) {

    //             cout << "found in directory!!!!!!!! " << filename << " " << pageNumber << " " << idx << endl;
    //             throw runtime_error("broken");
    //         }
    //         tmp = tmp->next;
    //     }
    // }

    cout << "out of while" << endl;
    if (!read_from_storage)
    {
        cout << "returning because flag is set" << endl;
        return nullopt;
    }
    cout << "not in table" << endl;
    // If we reach here, the page is not in the hash table. Try and read it from the file.
    cout << "reading page " << pageNumber << " from file " << filename << endl;
    optional<vector<pair<int, int>>> page = this->io_handler->readPageFromFile(filename, pageNumber);
    cout << "got page " << pageNumber << endl;
    if (page.has_value())
    {
        this->putPage(filename, pageNumber, page.value());

        return page;
    }

    return nullopt;
}

void ExtHash::evict()
{
    while (this->clock[this->curr_hand] != nullptr && this->clock[this->curr_hand]->recently_used == true)
    {
        this->clock[this->curr_hand]->recently_used = false;
        this->curr_hand = (this->curr_hand + 1) % clock.size();
    }

    shared_ptr<frame> evicting = this->clock[this->curr_hand];
    if (evicting == nullptr)
    {
        return;
    }

    int idx = GetIndexFromHashAndBitSize(evicting->hash, this->directory_bit_size);
    if (this->directory[idx] == evicting)
    {
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
    }
    this->clock[this->curr_hand] = nullptr;
    if (evicting->dirty)
    {
        this->io_handler->writeToFile(evicting->filename, evicting->data, evicting->pageNumber);
    }
}