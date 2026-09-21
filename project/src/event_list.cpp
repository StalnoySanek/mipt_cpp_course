#include "event_list.h"

namespace nano_edr
{
EventList::~EventList()
{
    ListClear(this);
}

void ListPopFront(EventList* list)
{
    if (list->size == 0) return;
    EventNode* tmp = list->head->next;
    delete list->head;
    list->head = tmp;
    list->size--;

    if (list->size == 0)
        list->tail = nullptr;
}

void ListPushBack(EventList* list, const Event* event)
{
    if (list->size == 0)
    {
        list->head = new EventNode(*event, nullptr);
        list->tail = list->head;
    }
    else
    {
        list->tail->next = new EventNode(*event, nullptr);
        list->tail = list->tail->next;
    }
    list->size++;

    if (list->capacity != 0 && list->size > list->capacity)
        ListPopFront(list);
}

void ListClear(EventList* list)
{
    while (list->head)
        ListPopFront(list);
}
}  // namespace nano_edr
