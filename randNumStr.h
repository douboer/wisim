/*
 * Add encryptStr() and decryptStr() functions
 * It's a simplify way to hide USB_SN string
 * - Chengan 9/12/2008
 */

#include <list>
#include <string>

#define MAXLEN 50
#define INTEVAL 5

#ifndef RAND_NUM_STR_H
#define RAND_NUM_STR_H

using namespace std;

int GenNum(const int range);
string GenNumOfString(const int len);
string GenLetterOfString(const int len);
string GenMixedString(const int len);
list<string> encryptStr(string instr);
string decryptStr(list<string> lst);
string decryptStr(string str);
void ExportToFile(const list<string>& lst, const string& strFilename);

#endif
