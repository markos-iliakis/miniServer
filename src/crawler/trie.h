#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

typedef struct plistNode{
	int file_index;
	int* line;
	int df;
	char path[SIZE_OF_ADDRESS];
    struct plistNode* next;
}plistNode;

typedef struct Plist{
    plistNode* head;
}Plist;

typedef struct trieNode{
    char letter;
    int End;
    Plist* list;
    struct trieNode* next;
    struct trieNode* child;
}trieNode;

typedef struct Trie{
    int letters;
	int words;
    trieNode* head;
}Trie;

Trie* TrieInit(void);
trieNode* trieNodeInit(void);
void Insert(Trie* root, char* key, char* path, int line, int file_index);
int Search(Trie* root, char* key, Plist** lroot);
void printTrie(trieNode* head);
void printAll(trieNode* head, char* buffer, int level);
void TrieDestroy(trieNode* head);

Plist* PlistInit(void);
plistNode* plistNodeInit(int line, char* path, int file_index);
void printList(Plist* lroot);
void ListDestroy(plistNode* p);
