#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "aesround.h"
#include "tree.h"

struct tree *inittree(void)
{
	struct tree *treeinstance = malloc(sizeof(*treeinstance));
	treeinstance->tree = NULL;
	treeinstance->NbrOfElm = 0;
	return treeinstance;
}
void inittreeentry(struct treeentry *_treeentry,uint8_t *value)
{
	_treeentry->childRight = NULL;
	_treeentry->childLeft = NULL;
	for(int i=0;i<16;++i)
		_treeentry->lastkey[i] = value[i];
}
void addElmToTree(struct tree *treeinstance,struct treeentry *treeentryinstance,uint8_t *value,int rightside)
{
  assert(treeinstance != NULL);
  assert(rightside == -1 || rightside == 0 || rightside == 1);
  if(treeentryinstance == NULL)
  {
    assert(rightside == -1);
  }
  else
  {
    assert(rightside == 0 || rightside == 1);
  }
  if(rightside != -1)
  {
    assert(treeentryinstance != NULL);

  }


	struct treeentry *entryinstance = malloc(sizeof(*entryinstance));
	assert(entryinstance != NULL);
	inittreeentry(entryinstance,value);

	// Connection in the tree
	if(treeinstance->NbrOfElm == 0)
	{
		assert(treeentryinstance == NULL);
		treeinstance->tree = entryinstance;
		treeinstance->NbrOfElm = 1;//1 child
	}
	else
	{
		if(treeentryinstance->childRight != NULL && treeentryinstance->childLeft  !=NULL)//it was a child
			treeinstance->NbrOfElm--;
		if(rightside == 1)
		{
			treeentryinstance->childRight = entryinstance;
			treeinstance->NbrOfElm = 1;
		}
		else
		{
			treeentryinstance->childLeft = entryinstance;
			treeinstance->NbrOfElm = 1;
		}
			
	}
	treeinstance->NbrOfElm++;
}
void generateTreeRec(struct tree *t0,struct treeentry *t,int byte)
{
	assert(t0 != NULL);
	assert(t != NULL);
	uint8_t faultedvalue = rsbox[0x41];
	uint8_t Rightantecedant = rsbox[0x63];//rsbox[0x63];
	uint8_t Leftantecedant = faultedvalue;//rsbox[faultedvalue];
 assert(Rightantecedant != Leftantecedant);
 assert(Rightantecedant == 0);
	if(byte<16)
	{
  if(t->lastkey[byte] == faultedvalue)
		{
			uint8_t RightState[16];
			uint8_t LeftState[16];
			for(int j=0;j<16;++j)
			{
				RightState[j] = t->lastkey[j];
				LeftState[j] = t->lastkey[j];
			}
			RightState[byte] = Rightantecedant;
                        LeftState[byte] = Leftantecedant;
			addElmToTree(t0,t,RightState,1);
			addElmToTree(t0,t,LeftState,0);
			generateTreeRec(t0,t->childRight,byte+1);
			generateTreeRec(t0,t->childLeft,byte+1);
		}
		else
			generateTreeRec(t0,t,byte+1);
	}
}
struct tree* generateTree(uint8_t *subkeyround,int *slist)
{
	struct tree *mtree = inittree();
	assert(mtree != NULL);
	addElmToTree(mtree,NULL,subkeyround,-1);
	generateTreeRec(mtree,mtree->tree,0);
	*slist = mtree->NbrOfElm;
	return mtree;

}


struct list *newlist()
{
	struct list *l = malloc(sizeof(*l));
	l->nbrOfElm = 0;
	l->firstelm = NULL;
	l->lastelm = NULL;
	return l;
}
void addToList(struct list *l,struct treeentry *e)
{
	struct listentry *le = malloc(sizeof(*le));
	le->e = e;
	le->next = NULL;

	if(l->firstelm == NULL)
	{
		l->firstelm = le;
		l->lastelm = le;
		l->nbrOfElm = 1;
	}
	else
	{
		l->lastelm->next = le;
		l->lastelm = le;
		l->nbrOfElm++;
	}
}


void tree2listrec(struct list *mlist,struct treeentry *current)
{
	if(current->childRight != NULL || current->childLeft != NULL)
        {
                assert(current->childRight != NULL && current->childLeft != NULL);
                tree2listrec(mlist,current->childRight);
                tree2listrec(mlist,current->childLeft);
        }
	else
	{
		addToList(mlist,current);
	}

}

struct list* tree2list(struct tree *mtree)
{
	struct list *mlist = newlist();
	tree2listrec(mlist,mtree->tree);
	return mlist;

}

struct list *generateSimpleElmList(uint8_t *subkey)
{
  struct tree *mtree = inittree();
  addElmToTree(mtree,NULL,subkey,-1);
  return tree2list(mtree);
}
struct list *generateExtension(uint8_t *subkey)
{
	int s;
        struct tree *t = generateTree(subkey,&s);
        struct list* l = tree2list(t);
	return l;
}


void concatenateList(struct list *dst,struct list *src)
{
  if(dst->firstelm == NULL)
    dst->firstelm = src->firstelm;
  dst->lastelm = src->lastelm;
  dst->nbrOfElm = dst->nbrOfElm + src->nbrOfElm;

}
struct list *generateExtensionFromList(struct list *originList)
{
  struct list *retlist = newlist();

  struct listentry *current = originList->firstelm;
  while(current != NULL)
  {
    struct list *p = generateExtension(current->e->lastkey);
    concatenateList(retlist,p);
    current = current->next;
  }
  return retlist;

}
void printlist(char *label,struct list *list)
{
  printf("%s\n",label);
  struct listentry *current = list->firstelm;
  while(current != NULL)
  {
    for(int i=0;i<16;++i)
      printf("%02X:",current->e->lastkey[i]);
    printf("\n");
    current = current->next;
  }
  printf("\n");
  
}

int areListEntryEqual(struct listentry *current0,struct listentry *current1)
{
  while(current0 != NULL)
  {
    for(int i=0;i<16;++i)
    {
      if(current0->e->lastkey[i] != current1->e->lastkey[i])
        return 0;
    }
    current0=current0->next;
    current1=current1->next;
  }
  return 1;
}
int areListEqual(struct list *list0,struct list *list1)
{
  if(list0->nbrOfElm != list1->nbrOfElm)
  {
    return 0;
  }
  return areListEntryEqual(list0->firstelm,list1->firstelm);
}
