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
//
int match_1(char p, char c) {
    if ( p == '\0') return TRUE; 
    if ( c == '\0') return FALSE; 
    return  p ==  c; 
}
int match(char * , char * ); 

int match_q(char * pattern, char * text) {
    return (match_1( * pattern,  * text) && (match(pattern + 2, text + 1))) || match(pattern + 2, text); 
}
int match_s(char * pattern, char * text) {
    return (match_1( * pattern,  * text) && (match(pattern, text + 1))) || match(pattern + 2, text); 
}
int match_g(char * pattern, char * text){
    char * group_end = strchr(pattern, ')');
    size_t group_size = group_end - pattern-1;
    char * group_pattern = (char*)malloc(sizeof(group_size));
    char * group_text = (char*)malloc(sizeof(group_size));
    char * remaind_pattern;
    memcpy(group_pattern, pattern+1, group_size);
    memcpy(group_text, text, group_size);
    if (*(group_end + 1) =='?') {
        remaind_pattern = group_end+2;
        return  (match(group_pattern, group_text) && match(remaind_pattern, text + group_size)) ||
                 match(remaind_pattern,text);
    } else if (*(group_end+1) == '*') {
        remaind_pattern = group_end+2;
        return  (match(group_pattern, group_text) && match(pattern, text + group_size)) ||
                 match(remaind_pattern,text);
    } else {
        remaind_pattern = group_end + 1;
        return  match(group_pattern, group_text) && 
                 match(remaind_pattern, text + group_size);

    }
}
int match(char * pattern, char * text) {
    if ( * pattern == '\0') return TRUE;
    else if ( * pattern == '$' &&  * text  == '\0') {
        return TRUE; 
    } else if ( * (pattern + 1) == '?') {
        return match_q(pattern, text); 
    } else if ( * (pattern + 1) == '*') {
        return match_s(pattern, text); 
    }else if ( * pattern == '.' &&  * text != '\0' ) {
        return match(pattern + 1, text + 1);
    } else if ( * pattern == '(') {
        return match_g(pattern, text);
    } else return match_1( * pattern,  * text) && match(pattern + 1, text + 1); 
}
int search(char * pattern, char * text) {
    int i = 0; 
    if ( * pattern == '^') {
        return match(pattern + 1, text);
    } else {
        while  (* text != '\0') {
            if (match(pattern, text ++ )) return TRUE;
        }
        return FALSE;             
    }   
}
int main(int argc, char * argv[]) {
    char * pattern = argv[1]; 
    char * text = argv[2]; 
     printf("%d\n", search(pattern, text)); 
    // printf("%d\n", match(pattern, text)); 
}
