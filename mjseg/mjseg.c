#include "mjseg.h"
#include "mjlog.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

mjslist mjseg_segment(mjseg seg, char* str) {
  if (!seg || !str) return NULL;
  if (segment(seg->_sgObj, str, strlen(str), seg->_enc, seg->_ps, seg->_renode, &seg->_words)) {
    MJLOG_ERR("segment error");
    return NULL;
  }
  SEG_WORD_SEGMENT_T* words = seg->_words;
  mjslist slist = mjslist_new();
  if (!slist) {
    MJLOG_ERR("mjstrlist_new error");
    return NULL;
  }
  char result[1024] = {0};
  for(int i = 0; i < words->word_num; i++) {
    if (words->wordlist[i].wordLen == 1 && (str + words->wordlist[i].wordPos)[0] == ' ') continue;
    if (words->wordlist[i].postag_id == POS_TAG_W) continue;
    int len = 0;
    strcpy(result + len, "\"");
    len++;
    memcpy(result + len, str + words->wordlist[i].wordPos, words->wordlist[i].wordLen);
    len += words->wordlist[i].wordLen;
    strcpy(result + len, "\":");
    len += 2;
    len += sprintf(result + len, "\"%u|%u\"", words->wordlist[i].idf, words->wordlist[i].postag_id);
    result[len] = 0;
    mjslist_adds(slist, result);
  }
  return slist;
}

mjseg mjseg_new(const char* conf_file) {
  if (!conf_file) return NULL;
  // alloc mjseg struct
  mjseg seg = (mjseg) calloc(1, sizeof(struct mjseg));
  if (!seg) return NULL;
  // seg sg handle
  seg->_sgObj = initSegHandle(conf_file, 0);
  if (!seg->_sgObj) return NULL;
  // set pthread result Handle
  seg->_renode = initPthreadSegResultHandle();
  if (!seg->_renode) {
    freeSegHandle(seg->_sgObj);
    return NULL;
  }
  seg->_enc = 1;
  seg->_ps = 0;
  seg->_words = NULL;
  return seg;
}

bool mjseg_delete(mjseg seg) {
  if (!seg) return false;
  freePthreadSegResultHandle(seg->_renode);
  freeSegHandle(seg->_sgObj);
  return true;
}
