#include "event_list.h"

namespace nano_edr
{
void EventList::PushBack(const Event& event)
{
    ++size_;
    if (!tail_)
        tail_ = head_ = new EventNode(event);
    else
        tail_->next = new EventNode(event);

    if (capacity_ != 0 && size_ > capacity_)
        PopFront();
}

void EventList::PopFront()
{
    switch (size_)
    {
        case 0:
            break;
        case 1:
            delete head_;
            head_ = tail_ = nullptr;
            break;
        default:
            auto next = head_->next;
            delete head_;
            head_ = next;
            break;
    }
    --size_;
}

void EventList::Clear()
{
    while (size_)
        PopFront();
}

EventList::~EventList()
{
    Clear();
}
}  // namespace nano_edr
