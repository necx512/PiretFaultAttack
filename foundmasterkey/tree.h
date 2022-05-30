#ifndef _TREE_H
#define _TREE_H
#include <stdint.h>
struct treeentry
{
	uint8_t lastkey[16];
	struct treeentry *childRight;
	struct treeentry *childLeft;
};
struct tree
{
	struct treeentry *tree;
	int NbrOfElm;
};
struct listentry
{
	struct treeentry *e;
	struct listentry *next;
};
struct list
{
	struct listentry *firstelm;
	struct listentry *lastelm;
	int nbrOfElm;

};

struct tree *inittree(void);
void inittreeentry(struct treeentry *_treeentry,uint8_t *value);
void addElmToTree(struct tree *treeinstance,struct treeentry *treeentryinstance,uint8_t *value,int rightside);
void generateTreeRec(struct tree *t0,struct treeentry *t,int byte);
struct tree* generateTree(uint8_t *subkeyround,int *slist);
struct list *newlist();
void addToList(struct list *l,struct treeentry *e);
void tree2listrec(struct list *mlist,struct treeentry *current);
struct list* tree2list(struct tree *mtree);
struct list *generateSimpleElmList(uint8_t *subkey);
struct list *generateExtension(uint8_t *subkey);
struct list *generateExtensionFromList(struct list *originList);
void printlist(char *label,struct list *list);
void concatenateList(struct list *dst,struct list *src);
int areListEqual(struct list *list0,struct list *list1);
int areListEntryEqual(struct listentry *current0,struct listentry *current1);
#endif //_TREE_H
