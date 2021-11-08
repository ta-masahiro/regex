#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define TRUE (1)
#define FALSE (0)
// 簡易正規表現エンジン 
// 元ネタは-> https://postd.cc/build-your-own-regex/
// 上記に()の多重化、|、[]を付け加え、match部分のアドレスを返すように改定したもの
//
// ? 前の文字と0個か1個一致
// * 前の文字と0個以上一致
// . 任意の文字と1個一致
// | 並列選択
// [] いずれか1文字
// () グループ化
// ^ 文字の先頭で一致
// $ 文字の末尾で一致
//
// ver0.42 searchの構造体をやめた
// ver0.41 . のバグ修正
// ver0.40 []に対応  
// TODO:
// '(.*)*'が必ずsegfoとなるバグ修正('(.*)?'はOK)
// '((ABC)*ABC)*ABC'が'ABCABCABC'の前半 3文字としかmatchしないバグ修正（直せないと思う…）
// エスケープ文字対応(現状「?*.|{}()^$」を含む文字列が検索できないので実用できない)
// '+'対応(簡単だがA+はAA*で対応可能なので優先順序低くて可)
// [...-...]、[^...]への対応（ないと不便)
// {n,m}の対応(使ったことないので優先低い?)
// マルチバイト文字の対応（簡単にできそうな気もすれば作り直すレベルで難しいのかも…)
int match_1(char p, char c) {
    if ( p == '\0') return TRUE; 
    if ( c == '\0') return FALSE;
    if ( p == '.' ) return TRUE;
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
    char * remaind_pattern,*p;
    memset(group_end,'\0',sizeof(char));
    if (*(group_end + 1) =='?') {
        remaind_pattern = group_end+2;
        if ((p=match(pattern+1, text, gr_lvl)) != NULL) {
            if ((p=match(remaind_pattern, p, gr_lvl))!=NULL) {memset(group_end,')',sizeof(char)); return p;}
        }
        memset(group_end,')',sizeof(char));
        return match(remaind_pattern,text, gr_lvl);
    } else if (*(group_end+1) == '*') {
        remaind_pattern = group_end+2;
        if ((p=match(pattern+1, text, gr_lvl)) != NULL) {
            memset(group_end,')',sizeof(char));
            if ((p = match(pattern, p, gr_lvl)) != NULL) return p;
        }
        memset(group_end,')',sizeof(char));
        return match(remaind_pattern, text, gr_lvl);
    } else {
        remaind_pattern = group_end + 1;
        if ((p=match(pattern+1, text, gr_lvl)) != NULL) {memset(group_end,')',sizeof(char));return match(remaind_pattern, p, gr_lvl);}
        memset(group_end,')',sizeof(char));
        return NULL;
    }
}
char * match_b(char * pattern, char * text, int gr_lvl) {
    char * br_end = strchr(pattern, ']'); 
    char * remaind_pattern,*p, *pp;
    if (*(br_end + 1) =='?') {
        remaind_pattern = br_end+2;
        for(pp = pattern+1; pp < br_end; pp++) {
            if (match_1(*pp, *text)) {
                if ((p=match(remaind_pattern, text+1, gr_lvl))!=NULL)  return p;
            }
        }
        return match(remaind_pattern,text, gr_lvl);
    } else if (*(br_end+1) == '*') {
        remaind_pattern = br_end+2;
        for (pp = pattern + 1; pp < br_end; pp ++) {
            if (match_1(*pp, *text)) {
                if ((p = match(pattern, text + 1, gr_lvl)) != NULL)  return p;
            }
        }
        return match(remaind_pattern,text, gr_lvl);
    } else {
        remaind_pattern = br_end + 1;
        for (pp = pattern + 1; pp < br_end; pp ++) {
            if (match_1(*pp, *text)) return match(remaind_pattern, text+1, gr_lvl);
        }
        return NULL;
    }
}
char * search_1st_bar(char *src, int gr_lvl) {
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
int COUNT = 0;
char * match(char * pattern, char * text, int gr_lvl) {
    char * bar_pos, *left_pos = pattern, * bar_pattern, *p;
    size_t bar_size;
    COUNT++;
    if ( * pattern == '\0') return text;
    else if ( * pattern == '$' &&  * text  == '\0') {
        return text; 
    } else if ((bar_pos = search_1st_bar(pattern, gr_lvl)) != NULL) {
        bar_size = bar_pos - pattern;
        memset(bar_pos,'\0',sizeof(char));
        if ((p = match(pattern,text,gr_lvl)) != NULL) {memset(bar_pos,'|',sizeof(char)); return p;}
        memset(bar_pos,'|',sizeof(char));
        return match(bar_pos+1,text,gr_lvl);
    } else if ( * (pattern + 1) == '?') {
        return match_q(pattern, text, gr_lvl); 
    } else if ( * (pattern + 1) == '*') {
        return match_s(pattern, text, gr_lvl); 
    } else if ( * pattern == '(') {
        return match_g(pattern, text, gr_lvl + 1);
    } else if ( * pattern == '[') {
        return match_b(pattern, text, gr_lvl);
    } else if (match_1( * pattern,  * text)) return match(pattern + 1, text + 1, gr_lvl); 
    else return NULL;
}
void search(char * pattern, char * text, char **top, char **end) {
    char * p; 
    if ( * pattern == '^') {
        if ((p = match(pattern + 1, text, 0)) != NULL) {*top = text; *end = p;return;}
        else {*top=(char*)0; *end=(char*)0; return;}
    } else {
        while  (* text != '\0') {
            if (p = match(pattern, text ++ , 0)) {*top = text-1; *end = p;return;} 
        }
        *top = (char*)0;*end = (char*)0;
        return;             
    }   
}
int main(int argc, char * argv[]) {
    char * pattern = argv[1]; 
    char * text = argv[2]; 
    char * match_top,*match_end;
    search(pattern,text,&match_top, &match_end);
    if ((match_end - match_top) == 0) printf("Missed!: Count=%d\n",COUNT);
    else printf("match in %ld to %ld: Count = %d\n",match_top - text, match_end - text,COUNT);
}
