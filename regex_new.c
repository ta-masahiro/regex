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
// | 並列選択
// $ 文字の末尾で一致
// () グループ化
//
// ver0.30メモリ割り当てを使わずに実装  
// TODO:
// searchでわざわざ構造体を作って返すのをやめる
// +対応
// [...]、[...-...]、[^...]への対応
int match_1(char p, char c) {
    if ( p == '\0') return TRUE; 
    if ( c == '\0') return FALSE; 
    return  p ==  c; 
}
char * match(char * , char * , int); 

char * match_q(char * pattern, char * text, int gr_lvl) {
    char *p;
    if (match_1( * pattern,  * text)) {
        if ((p = match(pattern + 2, text + 1, gr_lvl)) != NULL) return p;
    }
    return match(pattern + 2, text, gr_lvl); 
}
char * match_s(char * pattern, char * text, int gr_lvl) {
    char *p;
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
    char * group_end = search_gr_end(pattern, gr_lvl); 
    size_t group_size = group_end - pattern-1;
    //char * group_pattern = (char*)malloc(sizeof(group_size));
    //char * group_pattern = (char*)malloc(sizeof(group_size));
    char * remaind_pattern,*p;
    //memcpy(group_pattern, pattern+1, group_size);
    memset(group_end,'\0',sizeof(char));
    if (*(group_end + 1) =='?') {
        remaind_pattern = group_end+2;
        //if ((p=match(group_pattern, text, gr_lvl)) != NULL) {
        if ((p=match(pattern+1, text, gr_lvl)) != NULL) {
            //if ((p=match(remaind_pattern, p, gr_lvl))!=NULL) {free(group_pattern); return p;}
            if ((p=match(remaind_pattern, p, gr_lvl))!=NULL) {memset(group_end,')',sizeof(char)); return p;}
        }
        //free(group_pattern);
        memset(group_end,')',sizeof(char));
        return match(remaind_pattern,text, gr_lvl);
    } else if (*(group_end+1) == '*') {
        remaind_pattern = group_end+2;
        //if ((p=match(group_pattern, text, gr_lvl)) != NULL) {
        if ((p=match(pattern+1, text, gr_lvl)) != NULL) {
            //if ((p = match(pattern, p, gr_lvl)) != NULL) {free(group_pattern); return p;}
            memset(group_end,')',sizeof(char));
            if ((p = match(pattern, p, gr_lvl)) != NULL) return p;
        }
        //free(group_pattern);
        memset(group_end,')',sizeof(char));
        return match(remaind_pattern,text, gr_lvl);
    } else {
        remaind_pattern = group_end + 1;
        //if ((p=match(group_pattern, text, gr_lvl)) != NULL) {free(group_pattern);return match(remaind_pattern, p, gr_lvl);}
        if ((p=match(pattern+1, text, gr_lvl)) != NULL) {memset(group_end,')',sizeof(char));return match(remaind_pattern, p, gr_lvl);}
        //free(group_pattern);
        memset(group_end,')',sizeof(char));
        return NULL;
    }
}
char * search_1st_bar(char *src, int gr_lvl) {
    // search '|' in same group level
    int lvl=gr_lvl;
    char * p = src;
    while (*p != '\0') {
        if (*p == '|') {
            if (gr_lvl == lvl) return p;
        } else if (*p == '(') {
            lvl++;
        } else if (*p == ')') {
            lvl--;
        }
        p++;
    }
    return NULL;
}
char * match(char * pattern, char * text, int gr_lvl) {
    char * bar_pos, *left_pos = pattern, * bar_pattern, *p;
    size_t bar_size;
    if ( * pattern == '\0') return text;
    else if ( * pattern == '$' &&  * text  == '\0') {
        return text; 
    } else if ((bar_pos = search_1st_bar(pattern, gr_lvl)) != NULL) {
        bar_size = bar_pos - pattern;
        //bar_pattern = (char *)malloc(sizeof(bar_size));
        //memcpy(bar_pattern, pattern, bar_size);
        memset(bar_pos,'\0',sizeof(char));
        //if ((p = match(bar_pattern,text,gr_lvl)) != NULL) {free(bar_pattern); return p;}
        if ((p = match(pattern,text,gr_lvl)) != NULL) {memset(bar_pos,'|',sizeof(char)); return p;}
        //free(bar_pattern);
        memset(bar_pos,'|',sizeof(char));
        return match(bar_pos+1,text,gr_lvl);
    } else if ( * (pattern + 1) == '?') {
        return match_q(pattern, text, gr_lvl); 
    } else if ( * (pattern + 1) == '*') {
        return match_s(pattern, text, gr_lvl); 
    }else if ( * pattern == '.' &&  * text != '\0' ) {
        return match(pattern + 1, text + 1, gr_lvl);
    } else if ( * pattern == '(') {
        return match_g(pattern, text, gr_lvl + 1);
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
        if ((p = match(pattern + 1, text, 0)) != NULL) return new_pos(text,p);
        else return NULL;
    } else {
        while  (* text != '\0') {
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
    if (m_pos==NULL) printf("Missed\n");
    else printf("match in %ld to %ld\n",m_pos->top - text,m_pos->end - text);
}
