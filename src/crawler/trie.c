#include "trie.h"

Trie* TrieInit(void){
    Trie* p = malloc(sizeof(Trie));
    p->letters = 0;
	p->words = 0;
    p->head = NULL;
    return p;
}

trieNode* trieNodeInit(void){
    trieNode* p = malloc(sizeof(trieNode));
    p->letter = '/';
    p->list = NULL;
    p->End = 0;
    p->next = NULL;
    p->child = NULL;
    return p;
}

void Insert(Trie* root, char* key, char* path, int line, int file_index){
    int i = 0, flag = 0;

    trieNode* p = NULL;
    trieNode* n = root->head;

    int found;
    int length = strlen(key);
    // if(length > root->maxwLength)
    //     root->maxwLength = length;
	root->letters += length + 1;
	root->words ++;

    // itterate through the letters of a word
    while(i<length) {
        found = 0;
        // printf("In letter %c\n", key[i]);
        // itterate through the brothers
        while(n != NULL){
            // printf("-----Examining %c\n", n->letter);
            // if we found the letter we are seeking
            if(key[i] == n->letter){

                // if(n->child == NULL)
                //     printf("p : %c n : %c n->child : NULL\n", p->letter, n->letter);
                // else
                //     printf("p : %c n : %c n->child : %c\n", p->letter, n->letter, n->child->letter);

                p = n;
                n = n->child;
                found = 1;
                flag = 0;   // p father of n
                break;
            }

            // else move to the brother
            // if(n->next == NULL)
            //     printf("p : %c n : %c n->next : NULL\n", p->letter, n->letter);
            // else
            //     printf("p : %c n : %c n->next : %c\n", p->letter, n->letter, n->next->letter);

            p = n;
            n = n->next;
            flag = 1;    //p brother of n
        }

        if(!found) break;
        i++;
    }
	// printf("found %d\n", found);
    // append new nodes for the remaining characters
    if(!found){
        while(i < length){

            // create the new node
            n = trieNodeInit();
            n->letter = key[i];

            // set it as child or brother
            if (flag) {
                p->next = n;
                // printf("brother of %c created\n", p->letter);
            }
            else{
                if(p == NULL)
                    root->head = n;
                else
                    p->child = n;
                // printf("child of %c created\n", p->letter);
            }

            // all the next nodes will be children recursively
            flag = 0;

            if(i == length-1){
                n->End = 1;
                break;
            }

            p = n;
            n = n->child;
            i++;
        }
    }

    if(!found)
        p = n;

    if(p->list == NULL){
        p->list = PlistInit();
    }

    Plist* lroot = p->list;
    plistNode* l = lroot->head;
    plistNode* l1 = NULL;
    found = 0;

    // make the list
    while(l != NULL){
		// printf("compare %s with\n%s", l->path, path);
		if(strcmp(l->path, path) != 0){
            l1 = l;
            l = l->next;
        }
        else{
            found = 1;
			l->df++;
			l->line = realloc(l->line, l->df * sizeof(int));
			l->line[l->df-1] = line;
            break;
        }
    }

	// if(found) printf("match !\n");
	// getchar();
    if(!found){
        // printf("word not found again in this text\n");
        l = plistNodeInit(line, path, file_index);

        if(l1 != NULL)
            l1->next = l;
        else
            lroot->head = l;
    }

    // printTrie(root->head);
    // getchar();
}

int Search(Trie* root, char* key, Plist** lroot){
    trieNode* p = NULL;
    // printf("1\n");
    trieNode* n = root->head;
    // printf("2\n");
    int length = strlen(key);
    int i = 0;
    int found = 0;

    while (i<length) {
        found = 0;

        while(n!=NULL){
			// printf("examining letter %c\n", n->letter);
            if(key[i]==n->letter){
                p = n;
                n = n->child;
                found = 1;
                break;
            }
            else{
                p = n;
                n = n->next;
            }
        }
        if(!found) break;
        i++;
    }
    
	if(p->list == NULL)
		found = 0;
    
    if(found){
        // printf("2\n");
        *lroot = p->list;
	}
	// printf("search finished\n");
    return found;
}

void printAll(trieNode* head, char* buffer, int level){

    trieNode* p = head;

    while(p != NULL){

        printf("%c", p->letter);
        buffer[level] = p->letter;
        // printf("\n%s\n", buffer);

        if (p->End){
            // printf(" %d\n", p->list->df);
            if(p->child != NULL){
                printf("%s", buffer);
            }
        }

        if(p->child != NULL){
            printAll(p->child, buffer, level+1);
        }

        buffer[level] = '\0';

        if(p->next != NULL){
            printf("%s", buffer);
            p = p->next;
        }
        else{
            return;
        }
    }
}

void printTrie(trieNode* head){

    trieNode* p = head;

    while(p != NULL){

        printf("%c->", p->letter);
        if (p->End)
            printList(p->list);

        if(p->child != NULL)
            printTrie(p->child);

        printf("\n");

        if(p->next != NULL){
            printf("%c | ", p->letter);
            p = p->next;
        }
        else{
            return;
        }
    }
}

Plist* PlistInit(void){
    Plist* p = malloc(sizeof(Plist));
    p->head = NULL;
}

plistNode* plistNodeInit(int line, char* path, int file_index){
    plistNode* p = malloc(sizeof(plistNode));

    p->line = malloc(sizeof(int));
	p->line[0] = line;
	p->df = 1;
	p->file_index = file_index;
	strcpy(p->path, path);
	// p->path[strlen(path)] = '\0';
	// printf("%s\n", p->path);
	// getchar();
    p->next = NULL;
    return p;
}

void printList(Plist* lroot){
    plistNode* p = lroot->head;

    if(p == NULL)
        printf("NULLL\n");

    while(p != NULL){
		printf("[%s |%d", p->path, p->line[0]);
		for (size_t i = 1; i < p->df; i++) {
			printf(",%d", p->line[i]);
		}
        printf("|%d]->", p->df);
        p = p->next;
    }
}

void TrieDestroy(trieNode* head){
	trieNode* p = head;

    if(p != NULL){

        if (p->list!=NULL){
            ListDestroy(p->list->head);
			free(p->list);
		}

        if(p->child != NULL)
            TrieDestroy(p->child);

        if(p->next != NULL){
            TrieDestroy(p->next);
        }

		free(p);
		p=NULL;
        return;
    }
}

void ListDestroy(plistNode* p){
	if(p!=NULL){
		if(p->next!=NULL)
			ListDestroy(p->next);
		free(p);
		p=NULL;
	}
}
