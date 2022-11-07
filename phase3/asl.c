
#include <stdio.h>
#include <limits.h>
#include "../h/pcb.h"
#include "../h/asl.h"
HIDDEN semd_t *semd_h, *semdFree_h = NULL;


HIDDEN semd_t *allocSem()
{
    if (semdFree_h == NULL)
        return NULL;
    semd_t *freed = semdFree_h;
    semdFree_h = semdFree_h->s_next;
    freed->s_next = NULL;
    freed->s_procQ = mkEmptyProcQ();
    freed->s_semAdd = NULL; 
    return freed;
}


HIDDEN void deallocSem(semd_t *sem)
{
    
    sem->s_next = semdFree_h;
    semdFree_h = sem;
    sem->s_semAdd = NULL;
}

HIDDEN semd_t *search(int *semAdd)
{
    semd_t *current = semd_h;

    while (semAdd > (current->s_next->s_semAdd))
    {
        current = current->s_next;
    }

    return current;
}


void initASL()
{
    static semd_t semdTable[MAXPROC + 2];
    int i;
    for (i = 0; i < MAXPROC; i++)
    {
        deallocSem(&semdTable[i + 1]);
    }
    semd_h = (&semdTable[0]);
    semd_h->s_semAdd = (int *)INT_MIN;
    semd_h->s_next = (&semdTable[MAXPROC + 1]);
    semd_h->s_next->s_semAdd = (int *)INT_MAX;
}


int insertBlocked(int *semAdd, pcb_PTR p)
{
    semd_t *parent = search(semAdd);
    if (parent->s_next->s_semAdd == semAdd)
    {
        p->p_semAdd = semAdd;
        insertProcQ(&(parent->s_next->s_procQ), p);
    }
    else
    {
        semd_t *sem = allocSem();
        if (sem == NULL)
        {
            return TRUE;
        }
        sem->s_semAdd = semAdd;
        sem->s_procQ = mkEmptyProcQ();
        p->p_semAdd = semAdd;
        insertProcQ(&(sem->s_procQ), p);
        sem->s_next = parent->s_next;
        parent->s_next = sem;
    }
    return FALSE;
}


pcb_PTR removeBlocked(int *semdAdd)
{
    semd_t *parent = search(semdAdd);
    if (parent->s_next->s_semAdd == semdAdd)
    {
        pcb_PTR remove = removeProcQ(&(parent->s_next->s_procQ));
        if (remove == NULL)
        {
            return NULL;
        }
        if (emptyProcQ(parent->s_next->s_procQ))
        {
            semd_t *removed = parent->s_next;
            parent->s_next = parent->s_next->s_next;
            deallocSem(removed);
        }
        return remove;
    }
    return NULL;
}


pcb_PTR outBlocked(pcb_PTR p)
{
    int *semdAdd = p->p_semAdd;
    semd_t *parent = search(semdAdd);
    if (parent->s_next->s_semAdd == semdAdd)
    {
        pcb_PTR remove = outProcQ(&(parent->s_next->s_procQ), p);
        if (remove == NULL)
        {
            return NULL;
        }
        
        if (emptyProcQ(parent->s_next->s_procQ))
        {
            semd_t *removed = parent->s_next;
            parent->s_next = parent->s_next->s_next;
            deallocSem(removed);
        }
        return remove;
    }
    return NULL;
}


pcb_PTR headBlocked(int *semAdd)
{
    semd_t *temp = search(semAdd);
    if (temp->s_next->s_semAdd == semAdd)
    {
        return headProcQ(temp->s_next->s_procQ);
    }
    return NULL;
}
