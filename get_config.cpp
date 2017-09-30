/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
//从网上拷贝的代码
#include "get_config.h"

#include <fstream>
#include <iostream>
using namespace std;

bool IsSpace(char c) {
	if (' ' == c || '\t' == c)
		return true;
	return false;
}

bool IsCommentChar(char c) {
	switch (c) {
		case COMMENT_CHAR:
			return true;
		default:
			return false;
	}
}

void Trim(string & str) {
	if (str.empty()) {
		return;
	}
	int i, start_pos, end_pos;
	for (i = 0; i < str.size(); ++i) {
		if (!IsSpace(str[i])) {
			break;
		}
	}
	if (i == str.size()) { // 全部是空白字符串
		str = "";
		return;
	}

	start_pos = i;

	for (i = str.size() - 1; i >= 0; --i) {
		if (!IsSpace(str[i])) {
			break;
		}
	}
	end_pos = i;

	str = str.substr(start_pos, end_pos - start_pos + 1);
}

bool AnalyseLine(const string & line, string & key, string & value) {
	if (line.empty())
		return false;
	int start_pos = 0, end_pos = line.size() - 1, pos;
	if ((pos = line.find(COMMENT_CHAR)) != -1) {
		if (0 == pos) { // 行的第一个字符就是注释字符
			return false;
		}
		end_pos = pos - 1;
	}
	string new_line = line.substr(start_pos, start_pos + 1 - end_pos); // 预处理，删除注释部分

	if ((pos = new_line.find('=')) == -1)
		return false; // 没有=号

	key = new_line.substr(0, pos);
	value = new_line.substr(pos + 1, end_pos + 1 - (pos + 1));

	Trim(key);
	if (key.empty()) {
		return false;
	}
	Trim(value);
	return true;
}

bool ReadConfig(const string & filename, map<string, string> & m) {
	m.clear();
	ifstream infile(filename.c_str());
	if (!infile) {
		cout << "file open error" << endl;
		return false;
	}
	string line, key, value;
	while (getline(infile, line)) {
		if (AnalyseLine(line, key, value)) {
			m[key] = value;
		}
	}

	infile.close();
	return true;
}

/*
 * 输出打印配置文件
 */
void PrintConfig(const map<string, string> & m) {
	map<string, string>::const_iterator mite = m.begin();
	cout << "============================================" << endl;
	cout << "###[key] = [value]####" << endl;
	for (; mite != m.end(); ++mite) {
		string key = mite->first;
		string value = mite->second;
		cout << "[" << key << "] = [" << value << "]" << endl;
		//cout << "[" << mite->first << "] = [" << mite->second << "]"<< endl;
	}
	cout << "==========print config data complete========" << endl;
}

