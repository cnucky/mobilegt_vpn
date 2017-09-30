/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   get_config.h
 * Author: lenovo-pc
 *
 * Created on 2017年8月19日, 下午9:37
 * 从网上拷贝的代码
 * 配置文件格式
 * key1=value1
 * key2=value2
 * #开头的为注释
 */

#ifndef GET_CONFIG_H
#define GET_CONFIG_H

#include <string>
#include <map>
using namespace std;
 
#define COMMENT_CHAR '#'
 
bool ReadConfig(const string & filename, map<string, string> & m);
void PrintConfig(const map<string, string> & m);
bool AnalyseLine(const string & line, string & key, string & value);
#endif /* GET_CONFIG_H */

