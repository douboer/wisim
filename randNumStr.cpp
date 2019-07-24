/*
 *
randNumStr.cpp: 产生随机字符串（纯数字、纯字母、或数字字母组合），
 并将生成的字符串保存到文本文件中

 Author: 方新苗
 Email:  smecf@163.com
 Date:   2008.05.11

 Add encryptStr() and decryptStr() functions
 It's a simplify way to hide USB_SN string
 - Chengan 9/12/2008
 */


#include <iostream>
#include <vector>
#include <time.h>
#include <stdlib.h>
#include <fstream>

#include "randNumStr.h"

using namespace std;

/* 
 * 功能：产生一个指定范围内的随机数
 * 参数：iRange-随机数范围
 * 返值：产生的随机数
 */
int GenNum(const int iRange)
{
    return (rand() % iRange);
}

/* 
 * 功能：产生一个指定位数的由纯数字组成的字符串
 * 参数：iLength-字符串长度
 * 返值：随机字符串
 */
string GenNumOfString(const int iLength)
{
    string str = "";
    char ch[2];
    for (int i=0; i<iLength; i++)
    {
        itoa(GenNum(10), ch, 10);
        str += ch;
    }

    return str;
}

/* 
 * 功能：产生一个指定位数的由纯字母组成的字符串(97~122)
 * 参数：iLength-字符串长度
 * 返值：随机字符串
 */
string GenLetterOfString(const int iLength)
{
    string str = "";
    for (int i=0; i<iLength; i++)
        str += (char)('A' + GenNum(52));

    return str;
}

/* 
 * 功能：产生一个指定位数的由数字和字母组成的字符串(数字和字母的位数随机)
 * 参数：iLength-字符串长度
 * 返值：随机字符串
 */
string GenMixedString(const int iLength)
{
    string str = "";
    for (int i=0; i<iLength; i++)
    {
        if (GenNum(2))
            str += GenNumOfString(1);       // 加一个数字字符
        else
            str += GenLetterOfString(1);    // 加一个字母字符
    }

    return str;
}

/* 
 * 功能：把USB_SN中的每个字符串放入链表的每个记录中
 * 参数：USB_SN字符串
 * 返值：包含USB_SN信息的链表
 */
list<string> encryptStr(string instr)
{
    int len = instr.length();
    list<string> lst;
    list<string>::iterator iter;

    int pos = 0;
    string str = "";
    //std::cout << "lines: " << len << std::endl;
    for (int i=0; i<len; i++)
    {
        int pos = ((i+1)*INTEVAL)%MAXLEN;
        //std::cout << pos << std::endl;
        str = "";
        str += GenMixedString(pos);
        str += instr.at(i);
        str += GenMixedString(MAXLEN-pos-1);
        lst.push_back(str);
    }

    return lst;
}

/* 
 * 功能：提取链表中每个记录中的一个字符，组成USB_SN
 * 参数：lst-字符串链表
 * 返值：USB_SN字符串
 */
string decryptStr(list<string> lst)
{
    string str = "";
    int i      = 0;
    int pos    = 0;
    list<string>::iterator iter;
    for (iter = lst.begin(); iter != lst.end(); iter++)
    {
        pos = ((i+1)*INTEVAL)%MAXLEN;
        str += (*iter).at(pos);
        i++;
    }
    //std::cout << std::endl;
    //std::cout << str << std::endl;

    return str;
}

/* 
 * 功能：提取字符串中的USB_SN信息
 * 参数：包含USB_SN信息的字符串
 * 返值：USB_SN字符串
 */
string decryptStr(string str)
{
    string sstr = "";
    int i      = 0;
    int pos    = 0;
    int numrec = str.length()/MAXLEN;

    //std::cout << std::endl;
    //std::cout << numrec << std::endl;
    for (int i = 0; i < numrec; i++)
    {
        pos = i*MAXLEN + ((i+1)*INTEVAL)%MAXLEN;

        //std::cout << std::endl;
        //std::cout << pos << std::endl;

        sstr += str.at(pos);
    }
    //std::cout << std::endl;
    //std::cout << sstr << std::endl;

    return sstr;
}

/* 
 * 功能：提取文件中的USB_SN信息
 * 参数：包含USB_SN信息的文件名
 * 返值：USB_SN字符串
 */
//string decryptStr(string fn)
//{
//    string str;
//
//    ifstream fin( fn.c_str());  
//
//    if( !fin ) 
//    {   
//        std::cout << "Error opening " << fn << " for input" << std::endl;
//        exit(-1);  
//    }
//
//    while(getline(fin, str))
//    {
//    }
//
//    return decryptStr(str);
//}

/* 
 * 功能：将链表中的字符串保存到文件
 * 参数：lst-字符串链表、strFilename-文件名
 * 返值：void
 */
void ExportToFile(const list<string>& lst, const string& strFilename)
{
    ofstream out;
    out.open(strFilename.c_str(), ios_base::trunc);
    if (!out)
        cerr << "指定的文件名无效!" << endl;

    list<string>::const_iterator iter;
    for (iter = lst.begin(); iter != lst.end(); iter++)
        out << *iter;

    out.close();
}

/*
 *
int main( int argc, char ** argv )
{
    srand((unsigned)time(NULL));// 随机数发生器

    std::cout << std::endl;
    std::cout << "=====================" << std::endl;
    list<string> lst;
    list<string>::iterator iter;
    for (int i=0; i<20; i++)
    {
        lst.push_back(GenMixedString(MAXLEN));
        std::cout << GenMixedString(MAXLEN) << std::endl;
    }
    string fn = "sn.dat";
    string str = "";
    ExportToFile(lst, fn);

    lst.clear();
    str = "USB\\Vid_090c&Pid_6200\\12345678901234567890";
    lst = encryptStr(str);
    ExportToFile(lst, fn);

    str = "";
    for (iter = lst.begin(); iter != lst.end(); iter++)
    {
        str += *iter;
        std::cout << *iter;
    }
    std::cout << std::endl;
    std::cout << "=====================" << std::endl;
    std::cout << str << std::endl;

    std::cout << "=====================" << std::endl;
    decryptStr(lst);

    std::cout << "=====================" << std::endl;
    std::cout << decryptStr(str) << std::endl;

    std::cout << "=====================" << std::endl;
    ifstream fin(fn.c_str());
    if( !fin ) 
    {   
        std::cout << "Error opening " << fn << " for input" << std::endl;
        exit(-1);  
    }
    while(getline(fin, str)){}
    std::cout << decryptStr(str) << std::endl;

    return (0);
}
*
*/
