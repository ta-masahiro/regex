#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define TRUE (1)
#define FALSE (0)
// 簡易正規表現エンジン 
//  https://postd.cc/build-your-own-regex/
//
// ? 前の文字と0個か1個一致
// * 前の文字と0個以上一致
// . 任意の文字と1個一致
// ^ 文字の先頭で一致
// $ 文字の末尾で一致
// () グループ化
int match_1(char p, char c) {
    if ( p == '\0') return TRUE; 
    if ( c == '\0') return FALSE; 
    return  p ==  c; 
}
char * match(char * , char * , int); 

char * match_q(char * pattern, char * text, int gr_lvl) {
    char *p;
    //return (match_1( * pattern,  * text) && (match(pattern + 2, text + 1, gr_lvl))) || match(pattern + 2, text, gr_lvl); 
    if (match_1( * pattern,  * text)) {
        if ((p = match(pattern + 2, text + 1, gr_lvl)) != NULL) return p;
    }
    return match(pattern + 2, text, gr_lvl); 
}
char * match_s(char * pattern, char * text, int gr_lvl) {
    char *p;
    //return (match_1( * pattern,  * text) && (match(pattern, text + 1, gr_lvl))) || match(pattern + 2, text, gr_lvl); 
    if (match_1( * pattern,  * text)) {
        if ((p=match(pattern, text + 1, gr_lvl)) != NULL) return p;
    }
    return match(pattern + 2, text, gr_lvl); 
}
char * search_gr_end(char * src, int gr_lvl) {
    char * p = src+1;
    int lvl = gr_lvl;  
    while ( * p != '\0' ) {
        if ( * p == '(') {
            lvl ++ ;
        } else if (  * p == ')') {
            if (lvl == gr_lvl) return p; 
            lvl -- ;
        }
        p ++ ; 
    }
    return p; 
}
char * match_g(char * pattern, char * text, int gr_lvl) {
    // char * group_end = strchr(pattern, ')');
    char * group_end = search_gr_end(pattern, gr_lvl); 
    size_t group_size = group_end - pattern-1;
    char * group_pattern = (char*)malloc(sizeof(group_size));
    char * group_text = (char*)malloc(sizeof(group_size));
    char * remaind_pattern,*p;
    memcpy(group_pattern, pattern+1, group_size);
    memcpy(group_text, text, group_size);
    //group_text=text;
    if (*(group_end + 1) =='?') {
        remaind_pattern = group_end+2;
        //return  (match(group_pattern, group_text, gr_lvl) && match(remaind_pattern, text + group_size, gr_lvl)) ||
        //         match(remaind_pattern,text, gr_lvl);
        if (match(group_pattern, group_text, gr_lvl) != NULL) {
            if ((p=match(remaind_pattern, text + group_size, gr_lvl))!=NULL) return p;
        }
        return match(remaind_pattern,text, gr_lvl);
    } else if (*(group_end+1) == '*') {
        remaind_pattern = group_end+2;
        //return  (match(group_pattern, group_text, gr_lvl) && match(pattern, text + group_size, gr_lvl)) ||
        //         match(remaind_pattern,text, gr_lvl);
        if (match(group_pattern, group_text, gr_lvl) != NULL) {
            if ((p = match(pattern, text + group_size, gr_lvl)) != NULL) return p;
        }
        return match(remaind_pattern,text, gr_lvl);
    } else {
        remaind_pattern = group_end + 1;
        //return  match(group_pattern, group_text, gr_lvl) && 
        //         match(remaind_pattern, text + group_size, gr_lvl);
        if (match(group_pattern, group_text, gr_lvl) != NULL) return match(remaind_pattern, text + group_size, gr_lvl);
        return NULL;
    }
}
char * match(char * pattern, char * text, int gr_lvl) {
    //if ( * pattern == '\0') return TRUE;
    if ( * pattern == '\0') return text;
    else if ( * pattern == '$' &&  * text  == '\0') {
        //return TRUE;
        return text; 
    } else if ( * (pattern + 1) == '?') {
        return match_q(pattern, text, gr_lvl); 
    } else if ( * (pattern + 1) == '*') {
        return match_s(pattern, text, gr_lvl); 
    }else if ( * pattern == '.' &&  * text != '\0' ) {
        return match(pattern + 1, text + 1, gr_lvl);
    } else if ( * pattern == '(') {
        return match_g(pattern, text, gr_lvl + 1);
    //} else return match_1( * pattern,  * text) && match(pattern + 1, text + 1, gr_lvl); 
    } else if (match_1( * pattern,  * text)) return match(pattern + 1, text + 1, gr_lvl); 
    else return NULL;
}

typedef struct {
    char * top;
    char * end;
} pos;
pos * new_pos(char * top, char * end) {
    pos * m_pos = (pos*)malloc(sizeof(pos));
    m_pos->top = top;
    m_pos->end = end ;
    return m_pos;
}
pos * search(char * pattern, char * text) {
    char * p; 
    if ( * pattern == '^') {
        //return match(pattern + 1, text, 0);
        if ((p = match(pattern + 1, text, 0)) != NULL) return new_pos(text,p);
        else return NULL;
    } else {
        while  (* text != '\0') {
            //if (match(pattern, text ++ , 0)) return TRUE;
            if (p = match(pattern, text ++ , 0)) return new_pos(text-1,p);
        }
        return FALSE;             
    }   
}
int main(int argc, char * argv[]) {
    char * pattern = argv[1]; 
    char * text = argv[2]; 
    char * match_top,*match_end;
    pos * m_pos = search(pattern,text);
    //printf("%d\n", search(pattern, text));
    if (m_pos==NULL) printf("Missed\n");
    else printf("match in %ld to %ld\n",m_pos->top - text,m_pos->end - text);
}
