#include <iostream>
#include <cassert>
#include <cstring>
#include "list.h"

int itemCompare(itemType item1, itemType item2)
{
    return std::strcmp(item1.titolo, item2.titolo);
}

NODE* createNode(itemType item)
{
    NODE* p = new NODE;
    p->item = item;
    p->next = nullptr;
    return p;
}

void deleteNode(NODE* p)
{
    delete p;
}

LIST NewList()
{
    return nullptr;
}

LIST DeleteList(LIST l)
{
    LIST tmp = l;

    while (!isEmpty(tmp))
    {
        NODE* todel = tmp;
        tmp = tmp->next;
        deleteNode(todel);
    }

    return NewList();
}

bool isEmpty(LIST l)
{
    return l == nullptr;
}

int getLength(LIST l)
{
    int size = 0;
    LIST tmp = l;

    while (!isEmpty(tmp))
    {
        ++size;
        tmp = tmp->next;
    }

    return size;
}

itemType getHead(LIST l)
{
    assert(!isEmpty(l));
    return l->item;
}

itemType getTail(LIST l)
{
    NODE* tmp = l;
    assert(!isEmpty(l));

    while (!isEmpty(tmp->next))
        tmp = tmp->next;

    return tmp->item;
}

itemType* Find(LIST l, itemType item)
{
    if (!isEmpty(l))
    {
        LIST tmp = l;
        itemType* p;

        while (!isEmpty(tmp) && (itemCompare(item, tmp->item) != 0))
            tmp = tmp->next;

        if (!isEmpty(tmp))
        {
            p = &(tmp->item);
            return p;
        }
    }

    return nullptr;
}

LIST EnqueueFirst(LIST l, itemType item)
{
    assert(false);

    // TODO

    return l;
}

LIST EnqueueLast(LIST l, itemType item)
{
    NODE* new_node = createNode(item);

    if (isEmpty(l))
    {
        l = new_node;
    }
    else
    {
        LIST tmp = l;
        while (!isEmpty(tmp->next))
            tmp = tmp->next;
        tmp->next = new_node;
    }

    return l;
}

LIST EnqueueOrdered(LIST l, itemType item)
{
    NODE* newNode = createNode(item);

    if (isEmpty(l))
    {
        l = newNode;
    }
    else if (itemCompare(l->item, item) > 0)
    {
        newNode->next = l;
        l = newNode;
    }
    else
    {
        LIST tmp = l;
        while (!isEmpty(tmp->next) && itemCompare(tmp->next->item, item) < 0)
            tmp = tmp->next;

        if (!isEmpty(tmp->next))
        {
            newNode->next = tmp->next;
            tmp->next = newNode;
        }
        else
        {
            tmp->next = newNode;
        }
    }

    return l;
}

LIST DequeueFirst(LIST l)
{
    if (!isEmpty(l))
    {
        NODE* toDel = l;
        l = l->next;
        deleteNode(toDel);
    }

    return l;
}

LIST DequeueLast(LIST l)
{
    if (!isEmpty(l))
    {
        assert(false);

        // TODO
    }

    return l;
}

LIST Dequeue(LIST l, itemType item)
{
    if (!isEmpty(l))
    {
        if (itemCompare(l->item, item) == 0)
        {
            NODE* todel = l;
            l = l->next;
            deleteNode(todel);
        }
        else
        {
            LIST tmp = l;

            while (!isEmpty(tmp->next) && itemCompare(tmp->next->item, item) != 0)
                tmp = tmp->next;

            if (!isEmpty(tmp->next))
            {
                NODE* todel = tmp->next;
                tmp->next = tmp->next->next;
                deleteNode(todel);
            }
        }
    }
    return l;
}

void PrintItem(itemType item)
{
    std::cout << item.quantita << " - \"" << item.titolo << "\"" << std::endl;
}

void PrintList(LIST l)
{
    LIST tmp = l;
    while (!isEmpty(tmp))
    {
        PrintItem(tmp->item);
        tmp = tmp->next;
    }
    std::cout << std::endl;
}
