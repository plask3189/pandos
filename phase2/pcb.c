#include <stdio.h>

#include "../h/const.h"
#include "../h/types.h"
#include "../h/pcb.h"
HIDDEN pcb_PTR pcb_free_h;


void initPcbs()
{
    static pcb_t pcbs[MAXPROC];
    pcb_free_h = NULL;
    int i = 0;
    for (i = 0; i < MAXPROC; i++)
    {
        freePcb(&(pcbs[i]));
    }
}


void insertProcQ(pcb_PTR *tp, pcb_PTR p)
{
    if(!emptyProcQ(p)){
    if (emptyProcQ(*tp))
    {
        p->p_next = p;
        p->p_prev = p;
    }
    else
    {
        pcb_PTR temp = (*tp);
        p->p_next = temp;
        p->p_prev = temp->p_prev;
        temp->p_prev = p;
        p->p_prev->p_next = p;
    }
    (*tp) = p;
    }
}


pcb_PTR removeProcQ(pcb_PTR *tp)
{
    if (emptyProcQ(*tp))
    {
        return NULL;
    }
    pcb_PTR tail = *tp;
    if (tail->p_prev == tail)
    {
        (*tp) = NULL;
        return tail;
    }
    else
    {

        pcb_PTR remove = tail->p_prev;
        remove->p_prev->p_next = remove->p_next;
        remove->p_next->p_prev = remove->p_prev;
        return remove;
    }
}


pcb_PTR outProcQ(pcb_PTR *tp, pcb_PTR p)
{
    pcb_PTR tail = *tp;
    if(emptyProcQ(p)) return NULL;
    if(emptyProcQ(tail)) return NULL;
    pcb_PTR temp = tail->p_next;
    while(temp != p && temp != tail)
    {
        temp = temp->p_next;
    }
    if(temp == p){
        p->p_next->p_prev = p->p_prev;
        p->p_prev->p_next = p->p_next;
        if(temp == tail && tail->p_next == tail)
        {
            (*tp) = NULL;
        }else if(temp == tail)
        {
            (*tp) = tail->p_next;
        }
        return temp;
    } else {
        return NULL;
    }
}

void insertChild(pcb_PTR parent, pcb_PTR p)
{
    if(!emptyProcQ(p)){
    if (emptyChild(parent))
    {
        parent->p_child = p;
        p->p_prnt = parent;
        p->p_sib = NULL;
    }
    else
    {
        p->p_sib = parent->p_child;
        p->p_prnt = parent;
        parent->p_child = p;
    }
}
}


pcb_PTR removeChild(pcb_PTR p)
{
    if (emptyChild(p))
    {
        return NULL;
    }
    else
    {
        pcb_PTR child = p->p_child;
        p->p_child = child->p_sib;
        return child;
    }
}

/*
* Make the pcb pointed to by p no longer the child of its parent. If
* the pcb pointed to by p has no parent, return NULL; otherwise, return
* p. Note that the element pointed to by p need not be the first child of
* its parent.
*/
pcb_PTR outChild(pcb_PTR p)
{
    if(p->p_prnt == NULL) return NULL;
    pcb_PTR parent = p->p_prnt;
    pcb_PTR currentChild = parent->p_child;
    pcb_PTR lastChild = NULL;
    if (currentChild == p)
    {
        return removeChild(parent);
    }
    while (currentChild != NULL)
    {
        if (currentChild == p)
        {
            lastChild->p_sib = p->p_sib;
            p->p_prnt = NULL;
            return p;
        }
        lastChild = currentChild;
        currentChild = currentChild->p_sib;
    }
    return NULL;
}


int emptyProcQ(pcb_PTR tp)
{
    return (tp == NULL);
}


pcb_PTR headProcQ(pcb_PTR tp)
{
    if (emptyProcQ(tp))
    {
        return NULL;
    }
    return tp->p_prev;
}


int emptyChild(pcb_PTR p)
{
    return (p->p_child == NULL);
}


pcb_PTR mkEmptyProcQ()
{
    return NULL;
}


void freePcb(pcb_PTR p)
{
    insertProcQ(&(pcb_free_h), p);
}


pcb_PTR allocPcb()
{
    pcb_PTR allocate = removeProcQ(&(pcb_free_h));
    if(allocate!=NULL){
        allocate->p_child = NULL;
        allocate->p_next = NULL;
        allocate->p_prnt = NULL;
        allocate->p_semAdd = NULL;
        allocate->p_sib = NULL;
        allocate->p_time = NULL;
        allocate->p_supportStruct = NULL;
    }
    
    return allocate;

}



