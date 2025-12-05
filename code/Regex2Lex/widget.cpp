/****************************************************
 * @Copyright © 2023-2024 LDL. All rights reserved.
 *
 * @FileName: widget.cpp
 * @Brief: 主程序
 * @Module Function:
 *
 * @Current Version: 1.1 (Qt6 Fix)
 * @Author: Lidaliang
 * @Modifier: Gemini
 * @Finished Date: 2025/11/20
 *
 * @Version History: 1.1 适配Qt6，移除QTextCodec，优化文件IO
 *
 ****************************************************/
#include "widget.h"
#include "ui_widget.h"
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QFileDialog>
// #include <QTextCodec> // Qt6已移除，不再需要
#include <QMessageBox>
#include <QHeaderView>   // 新增：用于操作表头大小调整
#include <QProcess>      // 新增：用于编译和运行
#include <iostream>
#include <map>
#include <vector>
#include <stack>
#include <unordered_map>
#include <queue>
#include <set>
#include <unordered_set>
#include <algorithm>
#include <string>
#include <sstream>
#include <fstream>

// 如果源文件本身是UTF-8，这一行通常不是必须的，但在Windows MSVC下有助于识别字符串字面量
#pragma execution_character_set("utf-8")

using namespace std;

// EPSILON定义
const char EPSILON = '#';

// 下面map防止字符冲突
// 符号->字符串map (用于显示)
map<char, string> m1 = { {(char)3, "+"},{(char)4, "|"},{(char)5, "("},{(char)6, ")"},{(char)7, "*"},{(char)8, "-"},{(char)16, "?"},{(char)17, "["},{(char)18, "]"},{ (char)19,"~"}, {(char)20,"\n"}};
// 字符串->符号map (用于转义特殊字符)
map<string, char> m2 = { {"\\+", (char)3 },{"\\|", (char)4 },{ "\\(", (char)5},{ "\\)" ,(char)6},{"\\*" ,(char)7},{"\\-", (char)8},{ "\\?",(char)16},{"\\[", (char)17},{"\\]", (char)18 }, {"\\~", (char)19} };

// 变量名->特殊字符映射 (动态生成，如 letter -> (char)100, digit -> (char)101)
map<string, char> varCharMap;
// 特殊字符->变量名映射 (反向映射，用于显示)
map<char, string> charVarMap;
// 下一个可用的特殊字符编码
char nextVarChar = (char)100;

// 变量定义表 (如 letter=[A-Za-z])
map<string, string> varDefMap;
// 需要生成DFA的正则表达式列表 (以_开头的)
vector<pair<string, string>> regexToGenerate; // <名称, 正则表达式>
// 单词编码表
map<string, int> tokenCodeMap;
// 多单词标记 (以S结尾的定义，值为true表示是多单词)
map<string, bool> multiTokenMap;
// 多单词的各个token列表
map<string, vector<string>> multiTokenList;

// 正则表达式行合集
QString regexLine[5];
// 生成NFA和DFA的正则表达式
string finalRegex;

// 关键词合集
set<string> keyWords;
// 操作符号map
map<string,string> opMap;

// 注释符号集合，0为开始符号，1为结束符号
string commentSymbol[2];

// 全局结点计数器
int nodeCount = 0;

// 是否忽略大小写（默认不忽略）
bool isLowerCase = false;

// 全局字符统计
set<char> nfaCharSet;
set<char> dfaCharSet;


Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , m_lexerPath()
    , m_exePath()
{
    ui->setupUi(this);
}

Widget::~Widget()
{
    delete ui;
}

/*
* @brief 模拟trim函数
* 用于结果展示
*/
string trim(const string& str)
{
    if (str.empty() || str == "\\n")
    {
        return str;
    }
    string ret(str);
    ret.erase(0, ret.find_first_not_of("\\"));
    ret.erase(ret.find_last_not_of("\\") + 1);
    return ret;
}

/*
* @brief set转string
* 用于结果展示
*/
string set2string(set<int> s)
{
    string result;

    for (int i : s) {
        result.append(to_string(i));
        result.append(",");
    }

    if (result.size() != 0)
        result.pop_back(); //弹出最后一个逗号

    return result;
}

/*
* @brief 获取关键词列表
*/
void getKeyWords(QString regex) {
    string r = regex.toStdString();
    QStringList t = regex.split("|");
    for (QString s : t) {
        keyWords.insert(s.toStdString());
    }
}

/*
* @brief 获取操作符号的名称
*/
string getOpName(QString regex1, QString regex2) {
    QStringList op = regex1.split("|");
    QStringList opName = regex2.split("|");
    if (op.size() != opName.size()) {
        return "操作符和操作符名称个数不一致！";
    }
    for (int i = 0; i < op.size(); i++) {
        opMap[op[i].toStdString()] = opName[i].toStdString();
    }
    return "";
}

/*
* @brief 获取注释符号
*/
string getCommentSymbol(QString& regex) {
    QStringList t = regex.split("~");
    if (t.size() != 2) return "注释输入格式错误";
    commentSymbol[0] = t[0].toStdString();
    commentSymbol[1] = t[1].toStdString();
    regex = t[0] + "~*" + t[1];
    return "";
}

/*
* @brief 判断是不是字符
* 由于前面已经对出现在下面的字符进行了希腊字母转换，所以一定不会出现下面的字符
*/
bool isChar(char c)
{
    if (!(c == '+' || c == '[' || c == ']' || c == '|' || c == '*' || c == '(' || c == ')' || c == '?' || c == '@'))
        return true;
    return false;
}

/*
* @brief 进一步处理正则表达式
* 处理[] + 添加连接符
*/
QString afterHandleRegex(QString regex)
{
    string regexStd = regex.toStdString();
    qDebug() << "enter handleRegex: " << regex;

    // 处理中括号
    string result;
    bool insideBrackets = false;
    string currentString;

    for (char c : regexStd) {
        if (c == ' ') continue;
        if (c == '[') {
            insideBrackets = true;
            currentString.push_back('(');
        }
        else if (c == ']') {
            insideBrackets = false;
            currentString.push_back(')');
            result += currentString;
            currentString.clear();
        }
        else if (insideBrackets) {
            if (currentString.length() > 1) {
                currentString.push_back('|');
            }
            currentString.push_back(c);
        }
        else {
            result.push_back(c);
        }
    }

    regexStd = result;

    //先处理+号
    for (int i = 0; i < regexStd.size(); i++)
    {
        if (regexStd[i] == '+')
        {
            int kcount = 0;
            int j = i;
            do
            {
                j--;
                if (regexStd[j] == ')')
                {
                    kcount++;
                }
                else if (regexStd[j] == '(')
                {
                    kcount--;
                }
            } while (kcount != 0);
            string str1 = regexStd.substr(0, j);
            string kstr = regexStd.substr(j, i - j);
            string str2 = regexStd.substr(i + 1, (regexStd.size() - i));
            regexStd = str1 + kstr + kstr + "*" + str2;
        }
    }

    for (int i = 0; i < regexStd.size() - 1; i++)
    {
        if (isChar(regexStd[i]) && isChar(regexStd[i + 1])
            || isChar(regexStd[i]) && regexStd[i + 1] == '('
            || regexStd[i] == ')' && isChar(regexStd[i + 1])
            || regexStd[i] == ')' && regexStd[i + 1] == '('
            || regexStd[i] == '*' && regexStd[i + 1] != ')' && regexStd[i + 1] != '|' && regexStd[i + 1] != '?'
            || regexStd[i] == '?' && regexStd[i + 1] != ')'
            || regexStd[i] == '+' && regexStd[i + 1] != ')')
        {
            string str1 = regexStd.substr(0, i + 1);
            string str2 = regexStd.substr(i + 1, (regexStd.size() - i));
            str1 += "@";
            regexStd = str1 + str2;
        }

    }
    return QString::fromStdString(regexStd);
}

/*
* @brief 对正则表达式进行处理
* 新格式：
* 变量定义: name=regex (如 letter=[A-Za-z], digit=[0-9])
* 生成DFA: _name数字=regex (如 _ID101=letter(letter|digit)*)
* 说明：
* (1) 通过命名中加下划线(_)来表示该正则表达式需要生成DFA图
* (2) 命名中的名字后的数值为对应单词的编码
* (3) 命名中数值后加S表示后面有多个单词，编码从该数值开始
* (4) 支持转义符号：\+ \| \( \) \* \- \? \[ \] \~
*/
string handleAllRegex(QString allRegex, bool isLowerCase) {
    // 清空变量表
    varDefMap.clear();
    regexToGenerate.clear();
    tokenCodeMap.clear();
    multiTokenMap.clear();
    multiTokenList.clear();

    // 不区分大小写的话，全部转为小写
    if (isLowerCase) {
        allRegex = allRegex.toLower();
    }

    // 将文本内容按行分割
    QStringList lines = allRegex.split("\n", Qt::SkipEmptyParts);

    if (lines.isEmpty()) {
        return "输入为空，请输入正则表达式！";
    }

    // 第一遍：解析变量定义和需要生成的正则表达式
    for (const QString& line : lines) {
        QString trimmedLine = line.trimmed();
        if (trimmedLine.isEmpty()) continue;

        // 查找等号位置（第一个等号）
        int eqPos = trimmedLine.indexOf('=');
        if (eqPos == -1) {
            return "格式错误：每行必须是 name=regex 格式，错误行: " + trimmedLine.toStdString();
        }

        QString name = trimmedLine.left(eqPos).trimmed();
        QString regex = trimmedLine.mid(eqPos + 1).trimmed();

        if (name.isEmpty() || regex.isEmpty()) {
            return "格式错误：名称或正则表达式为空，错误行: " + trimmedLine.toStdString();
        }

        string nameStr = name.toStdString();
        string regexStr = regex.toStdString();

        // 判断是否以_开头（需要生成DFA）
        if (nameStr[0] == '_') {
            // 提取名称和编码，格式: _NAME123 或 _NAME123S
            string pureName = nameStr.substr(1); // 去掉下划线
            string tokenName;
            int tokenCode = 0;
            bool hasS = false;

            // 查找数字开始位置
            size_t numStart = string::npos;
            for (size_t i = 0; i < pureName.size(); i++) {
                if (isdigit(pureName[i])) {
                    numStart = i;
                    break;
                }
            }

            if (numStart != string::npos) {
                tokenName = pureName.substr(0, numStart);
                string numPart = pureName.substr(numStart);
                // 检查是否以S结尾（多单词标记）
                if (!numPart.empty() && (numPart.back() == 'S' || numPart.back() == 's')) {
                    hasS = true;
                    numPart.pop_back();
                }
                if (!numPart.empty()) {
                    tokenCode = stoi(numPart);
                }
            } else {
                tokenName = pureName;
            }

            // 如果是多单词定义（以S结尾），解析各个单词
            if (hasS) {
                multiTokenMap[tokenName] = true;
                // 按 | 分割多个单词（注意空格）
                QStringList tokens = regex.split("|");
                vector<string> tokenList;
                for (const QString& t : tokens) {
                    QString trimmed = t.trimmed();
                    if (!trimmed.isEmpty()) {
                        tokenList.push_back(trimmed.toStdString());
                    }
                }
                multiTokenList[tokenName] = tokenList;
                qDebug() << "多单词定义: " << QString::fromStdString(tokenName)
                         << " 起始编码: " << tokenCode
                         << " 单词数: " << tokenList.size();
                for (size_t i = 0; i < tokenList.size(); i++) {
                    qDebug() << "  单词" << i << ": " << QString::fromStdString(tokenList[i])
                             << " 编码: " << (tokenCode + (int)i);
                }
            }

            regexToGenerate.push_back({tokenName, regexStr});
            tokenCodeMap[tokenName] = tokenCode;
            qDebug() << "需要生成DFA: " << QString::fromStdString(tokenName)
                     << " 编码: " << tokenCode
                     << " 多单词: " << hasS
                     << " 正则: " << QString::fromStdString(regexStr);
        } else {
            // 普通变量定义
            varDefMap[nameStr] = regexStr;
            qDebug() << "变量定义: " << QString::fromStdString(nameStr)
                     << " = " << QString::fromStdString(regexStr);
        }
    }

    if (regexToGenerate.empty()) {
        return "没有找到需要生成DFA的正则表达式（以_开头的定义）！";
    }

    // 为每个变量分配一个特殊字符
    varCharMap.clear();
    charVarMap.clear();
    nextVarChar = (char)100;
    for (const auto& var : varDefMap) {
        char c = nextVarChar++;
        varCharMap[var.first] = c;
        charVarMap[c] = var.first;
        m1[c] = var.first;  // 添加到显示映射
        qDebug() << "变量映射: " << QString::fromStdString(var.first)
                 << " -> char(" << (int)c << ")";
    }

    // 第二遍：将变量名替换为对应的特殊字符（不展开定义）
    auto replaceVariables = [](string regex, const map<string, char>& vars) -> string {
        string result = regex;
        // 按变量名长度降序排序，避免短变量名匹配到长变量名的一部分
        vector<pair<string, char>> sortedVars(vars.begin(), vars.end());
        sort(sortedVars.begin(), sortedVars.end(),
             [](const auto& a, const auto& b) { return a.first.length() > b.first.length(); });

        for (const auto& var : sortedVars) {
            size_t pos = 0;
            while ((pos = result.find(var.first, pos)) != string::npos) {
                // 检查是否是独立的变量名（前后不是字母数字下划线）
                bool validStart = (pos == 0) || (!isalnum(result[pos-1]) && result[pos-1] != '_');
                bool validEnd = (pos + var.first.length() >= result.length()) ||
                               (!isalnum(result[pos + var.first.length()]) && result[pos + var.first.length()] != '_');
                if (validStart && validEnd) {
                    result.replace(pos, var.first.length(), string(1, var.second));
                    pos += 1;
                } else {
                    pos++;
                }
            }
        }
        return result;
    };

    // 替换所有需要生成的正则表达式中的变量名为特殊字符
    QString combinedRegex;
    for (size_t i = 0; i < regexToGenerate.size(); i++) {
        string replaced = replaceVariables(regexToGenerate[i].second, varCharMap);
        // 去除空格
        replaced.erase(remove(replaced.begin(), replaced.end(), ' '), replaced.end());
        regexToGenerate[i].second = replaced;
        qDebug() << "替换后: " << QString::fromStdString(regexToGenerate[i].first)
                 << " = " << QString::fromStdString(replaced);

        if (i > 0) combinedRegex += "|";
        combinedRegex += "(" + QString::fromStdString(replaced) + ")";
    }

    // 处理转义字符
    for (const auto& pair : m2) {
        combinedRegex.replace(QString::fromStdString(pair.first), QString(pair.second));
    }

    finalRegex = combinedRegex.toStdString();
    qDebug() << "合并后的正则表达式：" << combinedRegex;

    finalRegex = afterHandleRegex(combinedRegex).toStdString();
    qDebug() << "处理后的正则表达式：" << QString::fromStdString(finalRegex);

    return "";
}

/*============================正则表达式转NFA==================================*/

struct nfaNode; // 声明一下，不然报错

/*
* @brief 结构体，NFA图的边
*/
struct nfaEdge
{
    char c;
    nfaNode* next;
};
/*
* @brief 结构体，NFA图的结点
*/
struct nfaNode
{
    int id; // 结点唯一编号
    bool isStart;   // 初态标识
    bool isEnd; // 终态标识
    vector<nfaEdge> edges;  // 边，用vector因为有可能一个结点有多条边可走
    nfaNode() {
        id = nodeCount++;
        isStart = false;
        isEnd = false;
    }
};

/*
* @brief 结构体，NFA图
*/
struct NFA
{
    nfaNode* start;
    nfaNode* end;
    NFA() {}
    NFA(nfaNode* s, nfaNode* e)
    {
        start = s;
        end = e;
    }
};

/*
* @brief 创建基本字符NFA
* 只包含一个字符的NFA图
*/
NFA CreateBasicNFA(char character) {
    nfaNode* start = new nfaNode();
    nfaNode* end = new nfaNode();

    start->isStart = true;
    end->isEnd = true;

    nfaEdge edge;
    edge.c = character;
    edge.next = end;
    start->edges.push_back(edge);

    NFA nfa(start, end);

    // 存入全局nfa字符set
    nfaCharSet.insert(character);
    // 存入全局dfa字符set
    dfaCharSet.insert(character);

    return nfa;
}

/*
* @brief 创建连接运算符的NFA图
*/
NFA CreateConcatenationNFA(NFA nfa1, NFA nfa2) {
    // 把nfa1的终止状态与nfa2的起始状态连接起来
    nfa1.end->isEnd = false;
    nfa2.start->isStart = false;

    nfaEdge edge;
    edge.c = EPSILON; // 这里用EPSILON表示空边
    edge.next = nfa2.start;
    nfa1.end->edges.push_back(edge);

    NFA nfa;
    nfa.start = nfa1.start;
    nfa.end = nfa2.end;

    return nfa;
}

/*
* @brief 创建选择运算符的NFA图
*/
NFA CreateUnionNFA(NFA nfa1, NFA nfa2) {
    nfaNode* start = new nfaNode();
    nfaNode* end = new nfaNode();

    start->isStart = true;
    end->isEnd = true;

    // 把新的初态与nfa1和nfa2的初态连接起来
    nfaEdge edge1;
    edge1.c = EPSILON;
    edge1.next = nfa1.start;
    start->edges.push_back(edge1);
    nfa1.start->isStart = false;    // 初态结束

    nfaEdge edge2;
    edge2.c = EPSILON;
    edge2.next = nfa2.start;
    start->edges.push_back(edge2);
    nfa2.start->isStart = false;    // 初态结束

    // 把nfa1和nfa2的终止状态与新的终止状态连接起来
    nfa1.end->isEnd = false;
    nfa2.end->isEnd = false;

    nfaEdge edge3;
    edge3.c = EPSILON;
    edge3.next = end;
    nfa1.end->edges.push_back(edge3);

    nfaEdge edge4;
    edge4.c = EPSILON;
    edge4.next = end;
    nfa2.end->edges.push_back(edge4);

    NFA nfa{ start , end };

    return nfa;
}

/*
* @brief 创建*运算符的NFA图
*/
NFA CreateZeroOrMoreNFA(NFA nfa1) {
    nfaNode* start = new nfaNode();
    nfaNode* end = new nfaNode();

    start->isStart = true;
    end->isEnd = true;

    // 把新的初态与nfa1的初态连接起来
    nfaEdge edge1;
    edge1.c = EPSILON;
    edge1.next = nfa1.start;
    start->edges.push_back(edge1);
    nfa1.start->isStart = false;    // 初态结束

    // 把新的初态与新的终止状态连接起来
    nfaEdge edge2;
    edge2.c = EPSILON;
    edge2.next = end;
    start->edges.push_back(edge2);

    // 把nfa1的终止状态与新的终止状态连接起来
    nfa1.end->isEnd = false;

    nfaEdge edge3;
    edge3.c = EPSILON;
    edge3.next = nfa1.start;
    nfa1.end->edges.push_back(edge3);

    nfaEdge edge4;
    edge4.c = EPSILON;
    edge4.next = end;
    nfa1.end->edges.push_back(edge4);

    NFA nfa{ start,end };

    return nfa;
}

/*
* @brief 创建？运算符的NFA图
*/
NFA CreateOptionalNFA(NFA nfa1) {
    nfaNode* start = new nfaNode();
    nfaNode* end = new nfaNode();

    start->isStart = true;
    end->isEnd = true;

    // 把新的初态与nfa1的初态连接起来
    nfaEdge edge1;
    edge1.c = EPSILON;
    edge1.next = nfa1.start;
    start->edges.push_back(edge1);
    nfa1.start->isStart = false;    // 初态结束

    // 把新的初态与新的终止状态连接起来
    nfaEdge edge2;
    edge2.c = EPSILON;
    edge2.next = end;
    start->edges.push_back(edge2);

    // 把nfa1的终止状态与新的终止状态连接起来
    nfa1.end->isEnd = false;

    nfaEdge edge3;
    edge3.c = EPSILON;
    edge3.next = end;
    nfa1.end->edges.push_back(edge3);

    NFA nfa(start, end);

    return nfa;
}

/*
* @brief 判断优先级
*/
int Precedence(char op) {
    if (op == '|') {
        return 1;  // 选择运算符 "|" 的优先级
    }
    else if (op == '@') {
        return 2;  // 连接运算符 "@" 的优先级
    }
    else if (op == '*' || op == '?') {
        return 3;  // 闭包运算符 "*"和 "?" 的优先级
    }
    else {
        return 0;  // 默认情况下，将其它字符的优先级设为0
    }
}

/*
* @brief 结构体，状态转换表单个结点
*/
struct statusTableNode
{
    string flag;  // 标记初态还是终态
    int id; // 唯一id值
    map<char, set<int>> m;  // 对应字符能到达的状态
    statusTableNode()
    {
        flag = ""; // 默认为空
    }
};

// 状态转换表
unordered_map<int, statusTableNode> statusTable;
// statusTable插入顺序记录，方便后续输出
vector<int> insertionOrder;
set<int> startNFAstatus;
set<int> endNFAstatus;

/*
* @brief 生成状态转换表
* 使用DFS算法
*/
void createNFAStatusTable(NFA& nfa)
{
    stack<nfaNode*> nfaStack;
    set<nfaNode*> visitedNodes;

    // 初态
    nfaNode* startNode = nfa.start;
    statusTableNode startStatusNode;
    startStatusNode.flag = '-'; // -表示初态
    startStatusNode.id = startNode->id;
    statusTable[startNode->id] = startStatusNode;
    insertionOrder.push_back(startNode->id);
    startNFAstatus.insert(startNode->id);

    nfaStack.push(startNode);

    while (!nfaStack.empty()) {
        nfaNode* currentNode = nfaStack.top();
        nfaStack.pop();
        visitedNodes.insert(currentNode);

        for (nfaEdge edge : currentNode->edges) {
            char transitionChar = edge.c;
            nfaNode* nextNode = edge.next;

            // 记录状态转换信息
            statusTable[currentNode->id].m[transitionChar].insert(nextNode->id);

            // 如果下一个状态未被访问，将其加入堆栈
            if (visitedNodes.find(nextNode) == visitedNodes.end()) {
                nfaStack.push(nextNode);

                // 记录状态信息
                statusTableNode nextStatus;
                nextStatus.id = nextNode->id;
                if (nextNode->isStart) {
                    nextStatus.flag += '-'; // -表示初态
                    startNFAstatus.insert(nextStatus.id);
                }
                else if (nextNode->isEnd) {
                    nextStatus.flag += '+'; // +表示终态
                    endNFAstatus.insert(nextStatus.id);
                }
                statusTable[nextNode->id] = nextStatus;
                // 记录插入顺序（排除终态）
                if (!nextNode->isEnd)
                {
                    insertionOrder.push_back(nextNode->id);
                }
            }
        }
    }

    // 顺序表才插入终态
    nfaNode* endNode = nfa.end;
    insertionOrder.push_back(endNode->id);
}

// 测试输出NFA状态转换表程序（debug使用）
void printStatusTable() {
    // 打印状态表按照插入顺序
    for (int id : insertionOrder) {
        const statusTableNode& node = statusTable[id];
        qDebug() << "Node ID: " << node.id << ", Flag: " << QString::fromStdString(node.flag) << "\n";

        for (const auto& entry : node.m) {
            char transitionChar = entry.first;
            const std::set<int>& targetStates = entry.second;

            qDebug() << "  Transition: " << transitionChar << " -> {";
            for (int targetState : targetStates) {
                qDebug() << targetState << " ";
            }
            qDebug() << "}\n";
        }
    }
}


/*
* @brief 正则表达式转NFA入口
*/
NFA regex2NFA(string regex)
{
    // 双栈法，创建两个栈opStack（运算符栈）,nfaStack（nfa图栈）
    stack<char> opStack;
    stack<NFA> nfaStack;

    // 对表达式进行类似于逆波兰表达式处理
    // 运算符：| @（） ？ +  *
    for (char c : regex)
    {
        switch (c)
        {
        case ' ': // 空格跳过
            break;
        case '(':
            opStack.push(c);
            break;
        case ')':
            while (!opStack.empty() && opStack.top() != '(')
            {
                // 处理栈顶运算符，构建NFA图，并将结果入栈
                char op = opStack.top();
                opStack.pop();

                if (op == '|') {
                    // 处理并构建"|"运算符
                    NFA nfa2 = nfaStack.top();
                    nfaStack.pop();
                    NFA nfa1 = nfaStack.top();
                    nfaStack.pop();

                    // 创建新的NFA，表示nfa1 | nfa2
                    NFA resultNFA = CreateUnionNFA(nfa1, nfa2);
                    nfaStack.push(resultNFA);
                }
                else if (op == '@') {
                    // 处理并构建"."运算符
                    NFA nfa2 = nfaStack.top();
                    nfaStack.pop();
                    NFA nfa1 = nfaStack.top();
                    nfaStack.pop();

                    // 创建新的NFA，表示nfa1 . nfa2
                    NFA resultNFA = CreateConcatenationNFA(nfa1, nfa2);
                    nfaStack.push(resultNFA);
                }
            }
            if (opStack.empty())
            {
                qDebug() << "括号未闭合，请检查正则表达式！";
                exit(-1);
            }
            else
            {
                opStack.pop(); // 弹出(
            }
            break;
        case '|':
        case '@':
            // 处理运算符 | 和 @
            while (!opStack.empty() && (opStack.top() == '|' || opStack.top() == '@') &&
                   Precedence(opStack.top()) >= Precedence(c))
            {
                char op = opStack.top();
                opStack.pop();

                // 处理栈顶运算符，构建NFA图，并将结果入栈
                if (op == '|') {
                    // 处理并构建"|"运算符
                    NFA nfa2 = nfaStack.top();
                    nfaStack.pop();
                    NFA nfa1 = nfaStack.top();
                    nfaStack.pop();

                    // 创建新的NFA，表示nfa1 | nfa2
                    NFA resultNFA = CreateUnionNFA(nfa1, nfa2);
                    nfaStack.push(resultNFA);
                }
                else if (op == '@') {
                    // 处理并构建"."运算符
                    NFA nfa2 = nfaStack.top();
                    nfaStack.pop();
                    NFA nfa1 = nfaStack.top();
                    nfaStack.pop();

                    // 创建新的 NFA，表示 nfa1 . nfa2
                    NFA resultNFA = CreateConcatenationNFA(nfa1, nfa2);
                    nfaStack.push(resultNFA);
                }
            }
            opStack.push(c);
            break;
        case '?':
        case '*':
            // 处理闭包运算符 ? + *
            // 从nfaStack弹出NFA，应用相应的闭包操作，并将结果入栈
            if (!nfaStack.empty()) {
                NFA nfa = nfaStack.top();
                nfaStack.pop();
                if (c == '?') {
                    // 处理 ?
                    NFA resultNFA = CreateOptionalNFA(nfa);
                    nfaStack.push(resultNFA);
                }
                else if (c == '*') {
                    // 处理 *
                    NFA resultNFA = CreateZeroOrMoreNFA(nfa);
                    nfaStack.push(resultNFA);
                }
            }
            else {
                qDebug() << "正则表达式语法错误：闭包操作没有NFA可用！";
                exit(-1);
            }
            break;
        default:
            // 处理字母字符
            NFA nfa = CreateBasicNFA(c); // 创建基本的字符NFA
            nfaStack.push(nfa);
            break;
        }

    }

    // 处理栈中剩余的运算符
    while (!opStack.empty())
    {
        char op = opStack.top();
        opStack.pop();

        if (op == '|' || op == '@')
        {
            // 处理并构建运算符 | 和 .
            if (nfaStack.size() < 2)
            {
                qDebug() << "正则表达式语法错误：不足以处理运算符 " << op << "！";
                exit(-1);
            }

            NFA nfa2 = nfaStack.top();
            nfaStack.pop();
            NFA nfa1 = nfaStack.top();
            nfaStack.pop();

            if (op == '|')
            {
                // 创建新的 NFA，表示 nfa1 | nfa2
                NFA resultNFA = CreateUnionNFA(nfa1, nfa2);
                nfaStack.push(resultNFA);
            }
            else if (op == '@')
            {
                // 创建新的 NFA，表示 nfa1 . nfa2
                NFA resultNFA = CreateConcatenationNFA(nfa1, nfa2);
                nfaStack.push(resultNFA);
            }
        }
        else
        {
            qDebug() << "正则表达式语法错误：未知的运算符 " << op << "！";
            exit(-1);
        }
    }

    // 最终的NFA图在nfaStack的顶部
    NFA result = nfaStack.top();
    qDebug() << "NFA图构建完毕";

    createNFAStatusTable(result);
    qDebug() << "状态转换表构建完毕";

    return result;
}

/*============================NFA转DFA==================================*/
//map<int, nfaNode> nfaStates;

// dfa节点
struct dfaNode
{
    string flag; // 是否包含终态（+）或初态（-）
    set<int> nfaStates; // 该DFA状态包含的NFA状态的集合
    map<char, set<int>> transitions; // 字符到下一状态的映射
    dfaNode() {
        flag = "";
    }
};
// dfa状态去重集
set<set<int>> dfaStatusSet;

// dfa最终结果
vector<dfaNode> dfaTable;

//下面用于DFA最小化
// dfa终态集合
set<int> dfaEndStatusSet;
// dfa非终态集合
set<int> dfaNotEndStatusSet;
// set对应序号MAP
map<set<int>, int> dfa2numberMap;
int startStaus;

// 判断是否含有初态终态，含有则返回对应字符串
string setHasStartOrEnd(set<int>& statusSet)
{
    string result = "";
    for (const int& element : startNFAstatus) {
        if (statusSet.count(element) > 0) {
            result += "-";
        }
    }

    for (const int& element : endNFAstatus) {
        if (statusSet.count(element) > 0) {
            result += "+";
        }
    }

    return result;
}

set<int> epsilonClosure(int id)
{
    set<int> eResult{ id };
    stack<int> stack;
    stack.push(id);

    while (!stack.empty())
    {
        int current = stack.top();
        stack.pop();

        set<int> eClosure = statusTable[current].m[EPSILON];
        for (auto t : eClosure)
        {
            if (eResult.find(t) == eResult.end())
            {
                eResult.insert(t);
                stack.push(t);
            }
        }
    }

    return eResult;
}

set<int> otherCharClosure(int id, char ch)
{
    set<int> otherResult{};
    set<int> processed;
    stack<int> stack;
    stack.push(id);

    while (!stack.empty())
    {
        int current = stack.top();
        stack.pop();

        if (processed.find(current) != processed.end())
            continue;

        processed.insert(current);

        set<int> otherClosure = statusTable[current].m[ch];
        for (auto o : otherClosure)
        {
            auto tmp = epsilonClosure(o);
            otherResult.insert(tmp.begin(), tmp.end());
            stack.push(o);
        }
    }

    return otherResult;
}



// DFA debug输出函数
void printDfaTable(const vector<dfaNode>& dfaTable) {
    for (size_t i = 0; i < dfaTable.size(); ++i) {
        qDebug() << "DFA Node " << i << " - Flag: " << QString::fromStdString(dfaTable[i].flag);
        qDebug() << "NFA States: ";
        for (const int state : dfaTable[i].nfaStates) {
            qDebug() << state;
        }
        qDebug() << "Transitions: ";
        for (const auto& transition : dfaTable[i].transitions) {
            qDebug() << "  Input: " << transition.first;
            qDebug() << "  Next States: ";
            for (const int nextState : transition.second) {
                qDebug() << nextState;
            }
        }
        qDebug() << "---------------------";
    }
}

void NFA2DFA(NFA& nfa)
{
    int dfaStatusCount = 1;
    auto start = nfa.start; // 获得NFA图的起始位置
    auto startId = start->id;   // 获得起始编号
    dfaNode startDFANode;
    startDFANode.nfaStates = epsilonClosure(startId); // 初始闭包
    startDFANode.flag = setHasStartOrEnd(startDFANode.nfaStates); // 判断初态终态
    deque<set<int>> newStatus{};
    dfa2numberMap[startDFANode.nfaStates] = dfaStatusCount;
    startStaus = dfaStatusCount;
    if (setHasStartOrEnd(startDFANode.nfaStates).find("+") != string::npos) {
        dfaEndStatusSet.insert(dfaStatusCount++);
    }
    else
    {
        dfaNotEndStatusSet.insert(dfaStatusCount++);
    }
    // 对每个字符进行遍历
    for (auto ch : dfaCharSet)
    {
        set<int> thisChClosure{};
        for (auto c : startDFANode.nfaStates)
        {
            set<int> tmp = otherCharClosure(c, ch);
            thisChClosure.insert(tmp.begin(), tmp.end());
        }
        if (thisChClosure.empty())  // 如果这个闭包是空集没必要继续下去了
        {
            continue;
        }
        int presize = dfaStatusSet.size();
        dfaStatusSet.insert(thisChClosure);
        int lastsize = dfaStatusSet.size();
        // 不管一不一样都是该节点这个字符的状态
        startDFANode.transitions[ch] = thisChClosure;
        // 如果大小不一样，证明是新状态
        if (lastsize > presize)
        {
            dfa2numberMap[thisChClosure] = dfaStatusCount;
            newStatus.push_back(thisChClosure);
            if (setHasStartOrEnd(thisChClosure).find("+") != string::npos) {
                dfaEndStatusSet.insert(dfaStatusCount++);
            }
            else
            {
                dfaNotEndStatusSet.insert(dfaStatusCount++);
            }

        }

    }
    dfaTable.push_back(startDFANode);

    // 对后面的新状态进行不停遍历
    while (!newStatus.empty())
    {
        // 拿出一个新状态
        set<int> ns = newStatus.front();
        newStatus.pop_front();
        dfaNode DFANode;
        DFANode.nfaStates = ns;  // 该节点状态集合
        DFANode.flag = setHasStartOrEnd(ns);

        for (auto ch : dfaCharSet)
        {

            set<int> thisChClosure{};
            for (auto c : ns)
            {
                set<int> tmp = otherCharClosure(c, ch);
                thisChClosure.insert(tmp.begin(), tmp.end());
            }
            if (thisChClosure.empty())  // 如果这个闭包是空集没必要继续下去了
            {
                continue;
            }
            int presize = dfaStatusSet.size();
            dfaStatusSet.insert(thisChClosure);
            int lastsize = dfaStatusSet.size();
            // 不管一不一样都是该节点这个字符的状态
            DFANode.transitions[ch] = thisChClosure;
            // 如果大小不一样，证明是新状态
            if (lastsize > presize)
            {
                dfa2numberMap[thisChClosure] = dfaStatusCount;
                newStatus.push_back(thisChClosure);
                if (setHasStartOrEnd(thisChClosure).find("+") != string::npos) {
                    dfaEndStatusSet.insert(dfaStatusCount++);
                }
                else
                {
                    dfaNotEndStatusSet.insert(dfaStatusCount++);
                }

            }

        }
        dfaTable.push_back(DFANode);

    }

    // dfa debug
    // printDfaTable(dfaTable);
}

/*============================DFA最小化==================================*/
/*
// dfa终态集合
set<int> dfaEndStatusSet;
// dfa非终态集合
set<int> dfaNotEndStatusSet;
// set对应序号MAP
map<set<int>, int> dfa2numberMap;
*/

// 判断是否含有初态终态，含有则返回对应字符串
string minSetHasStartOrEnd(set<int>& statusSet)
{
    string result = "";
    if (statusSet.count(startStaus) > 0) {
        result += "-";
    }

    for (const int& element : dfaEndStatusSet) {
        if (statusSet.count(element) > 0) {
            result += "+";
            break;  // 可能会有多个终态同时包含，我们只要一个
        }
    }

    return result;
}

// dfa最小化节点
struct dfaMinNode
{
    string flag; // 是否包含终态（+）或初态（-）
    int id;
    map<char, int> transitions; // 字符到下一状态的映射
    dfaMinNode() {
        flag = "";
    }
};

vector<dfaMinNode> dfaMinTable;

// 用于分割集合
vector<set<int>> divideVector;
// 存下标
map<int, int> dfaMinMap;


// 根据字符 ch 将状态集合 node 分成两个子集合
void splitSet(int i, char ch)
{
    set<int> result;
    auto& node = divideVector[i];
    int s = -2;

    for (auto state : node)
    {
        int thisNum;
        if (dfaTable[state - 1].transitions.find(ch) == dfaTable[state - 1].transitions.end())
        {
            thisNum = -1; // 空集
        }
        else
        {
            // 根据字符 ch 找到下一个状态
            int next_state = dfa2numberMap[dfaTable[state - 1].transitions[ch]];
            thisNum = dfaMinMap[next_state];    // 这个状态的下标是多少
        }

        if (s == -2)    // 初始下标
        {
            s = thisNum;
        }
        else if (thisNum != s)   // 如果下标不同，就是有问题，需要分出来
        {
            result.insert(state);
        }
    }

    // 删除要删除的元素
    for (int state : result) {
        node.erase(state);
    }

    // 都遍历完了，如果result不是空，证明有新的，加入vector中
    if (!result.empty())
    {
        divideVector.push_back(result);
        // 同时更新下标
        for (auto a : result)
        {
            dfaMinMap[a] = divideVector.size() - 1;
        }
    }

}

void DFAminimize()
{
    divideVector.clear();
    dfaMinMap.clear();

    // 存入非终态、终态集合
    if (dfaNotEndStatusSet.size() != 0)
    {
        divideVector.push_back(dfaNotEndStatusSet);
    }
    // 初始化map
    for (auto t : dfaNotEndStatusSet)
    {
        dfaMinMap[t] = divideVector.size() - 1;
    }

    divideVector.push_back(dfaEndStatusSet);

    for (auto t : dfaEndStatusSet)
    {
        dfaMinMap[t] = divideVector.size() - 1;
    }

    // 当flag为1时，一直循环
    int continueFlag = 1;

    while (continueFlag)
    {
        continueFlag = 0;
        int size1 = divideVector.size();

        for (int i = 0; i < size1; i++)
        {

            // 逐个字符尝试分割状态集合
            for (char ch : dfaCharSet)
            {
                splitSet(i, ch);
            }
        }
        int size2 = divideVector.size();
        if (size2 > size1)
        {
            continueFlag = 1;
        }
    }

    for (int dfaMinCount = 0; dfaMinCount < divideVector.size(); dfaMinCount++)
    {
        auto& v = divideVector[dfaMinCount];
        dfaMinNode d;
        d.flag = minSetHasStartOrEnd(v);
        d.id = dfaMinCount;
        // 逐个字符
        for (char ch : dfaCharSet)
        {
            if (v.size() == 0)
            {
                d.transitions[ch] = -1;   // 空集特殊判断
                continue;
            }
            int i = *(v.begin()); // 拿一个出来
            if (dfaTable[i - 1].transitions.find(ch) == dfaTable[i - 1].transitions.end())
            {
                d.transitions[ch] = -1;   // 空集特殊判断
                continue;
            }
            int next_state = dfa2numberMap[dfaTable[i - 1].transitions[ch]];
            int thisNum = dfaMinMap[next_state];    // 这个状态下标
            d.transitions[ch] = thisNum;
        }
        dfaMinTable.push_back(d);
    }

    // 输出 dfaMinTable
    for (const dfaMinNode& node : dfaMinTable) {
        qDebug() << "State " << node.id << ":";
        qDebug() << "Flag: " << QString::fromStdString(node.flag);

        for (const auto& entry : node.transitions) {
            qDebug() << entry.first << " -> " << entry.second;
        }
    }

    qDebug() << "DFA最小化完成！";
}

// 辅助函数：根据变量定义生成字符范围的 case 语句
void generateCasesForVarDef(const string& varDef, QString& codeStr, bool& isLetter, bool& isDigit)
{
    // 解析变量定义，如 [A-Za-z] 或 [0-9]
    if (varDef.empty()) return;

    size_t i = 0;
    while (i < varDef.size()) {
        if (varDef[i] == '[') {
            i++;
            continue;
        }
        if (varDef[i] == ']') {
            break;
        }

        // 检查是否是范围，如 A-Z
        if (i + 2 < varDef.size() && varDef[i + 1] == '-') {
            char start = varDef[i];
            char end = varDef[i + 2];
            for (char c = start; c <= end; c++) {
                codeStr += "\t\t\tcase \'" + QString(c) + "\':\n";
                if (isalpha(c)) isLetter = true;
                if (isdigit(c)) isDigit = true;
            }
            i += 3;
        } else {
            // 单个字符
            char c = varDef[i];
            if (c != '[' && c != ']') {
                codeStr += "\t\t\tcase \'" + QString(c) + "\':\n";
                if (isalpha(c)) isLetter = true;
                if (isdigit(c)) isDigit = true;
            }
            i++;
        }
    }
}

bool genLexCase(QList<QString> tmpList, QString& codeStr, int idx, bool flag)
{
    bool rFlag = false;
    for (int i = 0; i < tmpList.size(); i++)
    {
        QString tmpKey = tmpList[i];
        char ch = tmpKey.toUtf8().constData()[0];
        if (dfaMinTable[idx].transitions[ch] == -1) {
            continue;
        }

        bool isLetter = false;
        bool isDigit = false;

        // 检查是否是变量（动态映射的特殊字符）
        if (charVarMap.find(ch) != charVarMap.end()) {
            string varName = charVarMap[ch];
            if (varDefMap.find(varName) != varDefMap.end()) {
                string varDef = varDefMap[varName];
                generateCasesForVarDef(varDef, codeStr, isLetter, isDigit);
                codeStr.chop(1); // 去掉最后一个换行
                if (flag) {
                    if (isLetter) codeStr += "isIdentifier = true; ";
                    if (isDigit) codeStr += "isDigit = true; ";
                    codeStr += "state = " + QString::number(dfaMinTable[idx].transitions[ch]) + "; ";
                }
                codeStr += "break;\n";
                continue;
            }
        }

        // 处理转义字符（如 \+ \* 等）
        if (m1.find(ch) != m1.end()) {
            string keyRes = m1[ch];
            // 处理特殊字符的转义
            if (keyRes == "+") keyRes = "+";
            else if (keyRes == "*") keyRes = "*";
            else if (keyRes == "-") keyRes = "-";
            else if (keyRes == "|") keyRes = "|";
            else if (keyRes == "(") keyRes = "(";
            else if (keyRes == ")") keyRes = ")";
            else if (keyRes == "?") keyRes = "?";
            else if (keyRes == "[") keyRes = "[";
            else if (keyRes == "]") keyRes = "]";
            else if (keyRes == "~") {
                rFlag = true;
                continue;
            }
            codeStr += "\t\t\tcase \'" + QString::fromStdString(keyRes) + "\':";
        } else {
            // 普通字符
            codeStr += "\t\t\tcase \'" + QString(ch) + "\':";
        }

        if (flag) {
            codeStr += "state = " + QString::number(dfaMinTable[idx].transitions[ch]) + "; ";
        }
        codeStr += "break;\n";
    }
    return rFlag;
}

QString generateLexer(QString filePath)
{
    QString lexCode;

    // 生成完整的 TINY 语言词法分析器
    lexCode += R"(#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

// Token types
typedef enum {
    TOKEN_KEYWORD,
    TOKEN_ID,
    TOKEN_NUM,
    TOKEN_OPERATOR,
    TOKEN_DELIMITER,
    TOKEN_EOF,
    TOKEN_ERROR
} TokenType;

// Keywords (8 total, case-insensitive)
const char* keywords[] = {"if", "then", "else", "end", "repeat", "until", "read", "write"};
const int NUM_KEYWORDS = 8;

// Token counter
int tokenCount = 0;

// Convert string to lowercase
void toLowerStr(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

// Check if string is a keyword
bool isKeyword(const char* str) {
    char lower[256];
    strcpy(lower, str);
    toLowerStr(lower);
    for (int i = 0; i < NUM_KEYWORDS; i++) {
        if (strcmp(lower, keywords[i]) == 0) {
            return true;
        }
    }
    return false;
}

// Get keyword name (lowercase)
const char* getKeywordName(const char* str) {
    static char lower[256];
    strcpy(lower, str);
    toLowerStr(lower);
    return lower;
}

// Output a token
void outputToken(FILE* fp, TokenType type, const char* value) {
    tokenCount++;
    const char* typeName;
    switch (type) {
        case TOKEN_KEYWORD: typeName = "KEYWORD"; break;
        case TOKEN_ID: typeName = "ID"; break;
        case TOKEN_NUM: typeName = "NUM"; break;
        case TOKEN_OPERATOR: typeName = "OPERATOR"; break;
        case TOKEN_DELIMITER: typeName = "DELIMITER"; break;
        case TOKEN_EOF: typeName = "EOF"; break;
        default: typeName = "ERROR"; break;
    }
    fprintf(fp, "%d: %s, %s\n", tokenCount, typeName, value);
    printf("%d: %s, %s\n", tokenCount, typeName, value);
}

// Lexical analyzer
void analyze(FILE* input_fp, FILE* output_fp) {
    int c;
    char buffer[1024];
    int bufIdx;
    
    printf("\n=== Lexical Analysis Results ===\n\n");
    fprintf(output_fp, "=== Lexical Analysis Results ===\n\n");
    
    while ((c = fgetc(input_fp)) != EOF) {
        // Skip whitespace
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            continue;
        }
        
        // Handle comments { ... }
        if (c == '{') {
            while ((c = fgetc(input_fp)) != EOF && c != '}') {
                // Skip comment content
            }
            continue;
        }
        
        // Handle identifiers and keywords
        if (isalpha(c)) {
            bufIdx = 0;
            buffer[bufIdx++] = c;
            while ((c = fgetc(input_fp)) != EOF && (isalnum(c))) {
                buffer[bufIdx++] = c;
            }
            buffer[bufIdx] = '\0';
            if (c != EOF) ungetc(c, input_fp);
            
            if (isKeyword(buffer)) {
                outputToken(output_fp, TOKEN_KEYWORD, getKeywordName(buffer));
            } else {
                outputToken(output_fp, TOKEN_ID, buffer);
            }
            continue;
        }
        
        // Handle numbers
        if (isdigit(c)) {
            bufIdx = 0;
            buffer[bufIdx++] = c;
            while ((c = fgetc(input_fp)) != EOF && isdigit(c)) {
                buffer[bufIdx++] = c;
            }
            buffer[bufIdx] = '\0';
            if (c != EOF) ungetc(c, input_fp);
            
            outputToken(output_fp, TOKEN_NUM, buffer);
            continue;
        }
        
        // Handle operators and delimiters
        buffer[0] = c;
        buffer[1] = '\0';
        
        // Check for two-character operators
        int next = fgetc(input_fp);
        if (c == ':' && next == '=') {
            strcpy(buffer, ":=");
            outputToken(output_fp, TOKEN_OPERATOR, buffer);
        } else if (c == '<' && next == '=') {
            strcpy(buffer, "<=");
            outputToken(output_fp, TOKEN_OPERATOR, buffer);
        } else if (c == '>' && next == '=') {
            strcpy(buffer, ">=");
            outputToken(output_fp, TOKEN_OPERATOR, buffer);
        } else if (c == '<' && next == '>') {
            strcpy(buffer, "<>");
            outputToken(output_fp, TOKEN_OPERATOR, buffer);
        } else {
            if (next != EOF) ungetc(next, input_fp);
            
            // Single character operators
            if (c == '+' || c == '-' || c == '*' || c == '/' || 
                c == '%' || c == '^' || c == '<' || c == '>' || c == '=') {
                outputToken(output_fp, TOKEN_OPERATOR, buffer);
            }
            // Delimiters
            else if (c == ';' || c == '(' || c == ')') {
                outputToken(output_fp, TOKEN_DELIMITER, buffer);
            }
            // Unknown character - skip
            else {
                // Ignore unknown characters
            }
        }
    }
    
    printf("\nTokens saved to output file\n");
}

int main(int argc, char* argv[]) {
)";

    // 修改 main 函数，支持命令行参数
    lexCode += R"(
    if (argc < 2) {
        printf("Usage: %s <input.tny> [output.lex]\n", argv[0]);
        printf("Example: %s sample.tny\n", argv[0]);
        printf("         %s sample.tny result.lex\n", argv[0]);
        return 1;
    }
    
    const char* inputFile = argv[1];
    
    // 生成输出文件名：如果提供了第二个参数则使用，否则将 .tny 替换为 .lex
    char outputFile[1024];
    if (argc >= 3) {
        strcpy(outputFile, argv[2]);
    } else {
        strcpy(outputFile, inputFile);
        char* dot = strrchr(outputFile, '.');
        if (dot != NULL) {
            strcpy(dot, ".lex");
        } else {
            strcat(outputFile, ".lex");
        }
    }
    
    printf("Input file:  %s\n", inputFile);
    printf("Output file: %s\n", outputFile);
    
    FILE* input_fp = fopen(inputFile, "r");
    if (input_fp == NULL) {
        printf("Error: Cannot open input file: %s\n", inputFile);
        return 1;
    }
    
    FILE* output_fp = fopen(outputFile, "w");
    if (output_fp == NULL) {
        printf("Error: Cannot open output file: %s\n", outputFile);
        fclose(input_fp);
        return 1;
    }
    
    analyze(input_fp, output_fp);
    
    fclose(input_fp);
    fclose(output_fp);
    
    return 0;
}
)";

    return lexCode;
}

// 生成 Mini-C 语言词法分析器
QString generateMiniCLexer(QString filePath)
{
    Q_UNUSED(filePath);
    QString lexCode;

    lexCode += R"(#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

// Token types
typedef enum {
    TOKEN_KEYWORD,
    TOKEN_ID,
    TOKEN_NUM,
    TOKEN_FLOAT,
    TOKEN_OPERATOR,
    TOKEN_DELIMITER,
    TOKEN_EOF,
    TOKEN_ERROR
} TokenType;

// Mini-C Keywords (8 total, case-sensitive)
const char* keywords[] = {"else", "if", "int", "float", "real", "return", "void", "while", "for"};
const int NUM_KEYWORDS = 9;

// Token counter
int tokenCount = 0;

// Check if string is a keyword
bool isKeyword(const char* str) {
    for (int i = 0; i < NUM_KEYWORDS; i++) {
        if (strcmp(str, keywords[i]) == 0) {
            return true;
        }
    }
    return false;
}

// Output a token
void outputToken(FILE* fp, TokenType type, const char* value) {
    tokenCount++;
    const char* typeName;
    switch (type) {
        case TOKEN_KEYWORD: typeName = "KEYWORD"; break;
        case TOKEN_ID: typeName = "ID"; break;
        case TOKEN_NUM: typeName = "NUM"; break;
        case TOKEN_FLOAT: typeName = "FLOAT"; break;
        case TOKEN_OPERATOR: typeName = "OPERATOR"; break;
        case TOKEN_DELIMITER: typeName = "DELIMITER"; break;
        case TOKEN_EOF: typeName = "EOF"; break;
        default: typeName = "ERROR"; break;
    }
    fprintf(fp, "%d: %s, %s\n", tokenCount, typeName, value);
    printf("%d: %s, %s\n", tokenCount, typeName, value);
}

// Lexical analyzer for Mini-C
void analyze(FILE* input_fp, FILE* output_fp) {
    int c;
    char buffer[1024];
    int bufIdx;
    
    printf("\n=== Mini-C Lexical Analysis Results ===\n\n");
    fprintf(output_fp, "=== Mini-C Lexical Analysis Results ===\n\n");
    
    while ((c = fgetc(input_fp)) != EOF) {
        // Skip whitespace
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            continue;
        }
        
        // Handle line comments //
        if (c == '/') {
            int next = fgetc(input_fp);
            if (next == '/') {
                // Skip until end of line
                while ((c = fgetc(input_fp)) != EOF && c != '\n') {
                    // Skip comment content
                }
                continue;
            } else {
                // It's division operator
                if (next != EOF) ungetc(next, input_fp);
                buffer[0] = '/';
                buffer[1] = '\0';
                outputToken(output_fp, TOKEN_OPERATOR, buffer);
                continue;
            }
        }
        
        // Handle identifiers and keywords (start with letter or _)
        if (isalpha(c) || c == '_') {
            bufIdx = 0;
            buffer[bufIdx++] = c;
            while ((c = fgetc(input_fp)) != EOF && (isalnum(c) || c == '_')) {
                buffer[bufIdx++] = c;
            }
            buffer[bufIdx] = '\0';
            if (c != EOF) ungetc(c, input_fp);
            
            if (isKeyword(buffer)) {
                outputToken(output_fp, TOKEN_KEYWORD, buffer);
            } else {
                outputToken(output_fp, TOKEN_ID, buffer);
            }
            continue;
        }
        
        // Handle numbers (integer or float)
        if (isdigit(c)) {
            bufIdx = 0;
            buffer[bufIdx++] = c;
            bool isFloat = false;
            
            // Read integer part
            while ((c = fgetc(input_fp)) != EOF && isdigit(c)) {
                buffer[bufIdx++] = c;
            }
            
            // Check for decimal point
            if (c == '.') {
                int next = fgetc(input_fp);
                if (isdigit(next)) {
                    isFloat = true;
                    buffer[bufIdx++] = c;  // Add '.'
                    buffer[bufIdx++] = next;
                    // Read fractional part
                    while ((c = fgetc(input_fp)) != EOF && isdigit(c)) {
                        buffer[bufIdx++] = c;
                    }
                } else {
                    // Not a float, put back
                    if (next != EOF) ungetc(next, input_fp);
                    ungetc(c, input_fp);
                    c = '.';  // Will be processed later
                }
            }
            
            buffer[bufIdx] = '\0';
            if (c != EOF && c != '.') ungetc(c, input_fp);
            
            if (isFloat) {
                outputToken(output_fp, TOKEN_FLOAT, buffer);
            } else {
                outputToken(output_fp, TOKEN_NUM, buffer);
            }
            continue;
        }
        
        // Handle operators and delimiters
        buffer[0] = c;
        buffer[1] = '\0';
        
        // Check for two-character operators
        int next = fgetc(input_fp);
        
        if (c == '+' && next == '+') {
            strcpy(buffer, "++");
            outputToken(output_fp, TOKEN_OPERATOR, buffer);
        } else if (c == '-' && next == '-') {
            strcpy(buffer, "--");
            outputToken(output_fp, TOKEN_OPERATOR, buffer);
        } else if (c == '<' && next == '=') {
            strcpy(buffer, "<=");
            outputToken(output_fp, TOKEN_OPERATOR, buffer);
        } else if (c == '>' && next == '=') {
            strcpy(buffer, ">=");
            outputToken(output_fp, TOKEN_OPERATOR, buffer);
        } else if (c == '=' && next == '=') {
            strcpy(buffer, "==");
            outputToken(output_fp, TOKEN_OPERATOR, buffer);
        } else if (c == '!' && next == '=') {
            strcpy(buffer, "!=");
            outputToken(output_fp, TOKEN_OPERATOR, buffer);
        } else {
            if (next != EOF) ungetc(next, input_fp);
            
            // Single character operators
            if (c == '+' || c == '-' || c == '*' || c == '/' || 
                c == '%' || c == '^' || c == '<' || c == '>' || c == '=' || c == '.') {
                outputToken(output_fp, TOKEN_OPERATOR, buffer);
            }
            // Delimiters
            else if (c == ';' || c == ',' || c == '(' || c == ')' || 
                     c == '[' || c == ']' || c == '{' || c == '}') {
                outputToken(output_fp, TOKEN_DELIMITER, buffer);
            }
            // Unknown character - skip
            else {
                // Ignore unknown characters
            }
        }
    }
    
    printf("\nTokens saved to output file\n");
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Mini-C Lexical Analyzer\n");
        printf("Usage: %s <input.mc> [output.lex]\n", argv[0]);
        printf("Example: %s sample.mc\n", argv[0]);
        printf("         %s sample.mc result.lex\n", argv[0]);
        return 1;
    }
    
    const char* inputFile = argv[1];
    
    // Generate output filename
    char outputFile[1024];
    if (argc >= 3) {
        strcpy(outputFile, argv[2]);
    } else {
        strcpy(outputFile, inputFile);
        char* dot = strrchr(outputFile, '.');
        if (dot != NULL) {
            strcpy(dot, ".lex");
        } else {
            strcat(outputFile, ".lex");
        }
    }
    
    printf("Input file:  %s\n", inputFile);
    printf("Output file: %s\n", outputFile);
    
    FILE* input_fp = fopen(inputFile, "r");
    if (input_fp == NULL) {
        printf("Error: Cannot open input file: %s\n", inputFile);
        return 1;
    }
    
    FILE* output_fp = fopen(outputFile, "w");
    if (output_fp == NULL) {
        printf("Error: Cannot open output file: %s\n", outputFile);
        fclose(input_fp);
        return 1;
    }
    
    analyze(input_fp, output_fp);
    
    fclose(input_fp);
    fclose(output_fp);
    
    return 0;
}
)";

    return lexCode;
}


/*
* @brief 初始化函数
* 用于清空全局变量
*/
void init() {
    // 全局变量清空
    keyWords.clear();
    opMap.clear();
    finalRegex.clear();
    regexLine->clear();
    commentSymbol->clear();
    nodeCount = 0;
    nfaCharSet.clear();
    dfaCharSet.clear();
    statusTable.clear();
    insertionOrder.clear();
    startNFAstatus.clear();
    endNFAstatus.clear();
    dfaStatusSet.clear();
    dfaEndStatusSet.clear();
    dfaNotEndStatusSet.clear();
    dfaMinTable.clear();
    divideVector.clear();
    dfaMinMap.clear();
    dfaTable.clear();
    dfa2numberMap.clear();
    varDefMap.clear();
    regexToGenerate.clear();
    tokenCodeMap.clear();
    multiTokenMap.clear();
    multiTokenList.clear();
    varCharMap.clear();
    charVarMap.clear();
    // 清除动态添加的变量映射
    for (char c = 100; c < nextVarChar; c++) {
        m1.erase(c);
    }
    nextVarChar = (char)100;
    nfaCharSet.insert(EPSILON); // 放入epsilon
}

/*
* @brief 开始分析按钮
*/
void Widget::on_pushButton_clicked(){
    // 清空全局变量
    init();

    // 拿到所有的正则表达式
    QString allRegex = ui->plainTextEdit_2->toPlainText();

    isLowerCase = ui->checkBox->isChecked();
    qDebug() <<"是否区分大小写："<< isLowerCase;

    string result = handleAllRegex(allRegex, isLowerCase);
    // 如果字符串不为空就是报错了，退出
    if (!result.empty()) {
        QMessageBox::critical(this, "错误信息", QString::fromStdString(result));
        return;
    }

    //正则表达式转换成NFA图
    NFA nfa = regex2NFA(finalRegex);

    // NFA转DFA
    NFA2DFA(nfa);

    DFAminimize();

    QMessageBox::about(this, "提示", "分析成功！请点击其余按钮查看结果！");
}

/*
* @brief 生成NFA按钮
*/
void Widget::on_pushButton_4_clicked()
{
    ui->tableWidget->clearContents(); // 清除表格中的数据
    ui->tableWidget->setRowCount(0); // 清除所有行
    ui->tableWidget->setColumnCount(0); // 清除所有列
    // 设置列数
    int n = 2 + nfaCharSet.size(); // 默认两列：Flag 和 ID
    ui->tableWidget->setColumnCount(n);

    // 字符和第X列存起来对应
    map<char, int> headerCharNum;

    // 设置表头
    QStringList headerLabels;
    headerLabels << "标志" << "ID";
    int headerCount = 3;
    for (const auto& ch : nfaCharSet) {
        if (m1.find(ch) != m1.end()) {
            headerLabels << QString::fromStdString(trim(m1[ch]));
        }
        else {
            if (ch == '~') {
                char tt = commentSymbol[1][0];
                string res = commentSymbol[1];
                if (m1.find(tt) != m1.end()) {
                    res = trim(m1[tt]);
                }
                headerLabels << QString::fromStdString("非" + res);
            }
            else {
                headerLabels << QString(ch);
            }
        }

        headerCharNum[ch] = headerCount++;
    }
    ui->tableWidget->setHorizontalHeaderLabels(headerLabels);

    // 设置行数
    int rowCount = statusTable.size();
    ui->tableWidget->setRowCount(rowCount);

    // 填充数据
    int row = 0;
    for (auto id : insertionOrder) {
        const statusTableNode& node = statusTable[id];

        // Flag 列
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(node.flag)));

        // ID 列
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(QString::number(node.id)));

        // TransitionChar 列
        int col = 2;
        for (const auto& transitionEntry : node.m) {
            string resutlt = set2string(transitionEntry.second);

            // 放到指定列数据
            ui->tableWidget->setItem(row, headerCharNum[transitionEntry.first] - 1, new QTableWidgetItem(QString::fromStdString(resutlt)));
            col++;
        }

        row++;
    }

    // 调整列宽
    // 【QT6修复】添加 #include <QHeaderView> 以支持此调用
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // 显示表格
    ui->tableWidget->show();
}

/*
* @brief 生成DFA按钮
*/
void Widget::on_pushButton_3_clicked()
{
    ui->tableWidget->clearContents(); // 清除表格中的数据
    ui->tableWidget->setRowCount(0); // 清除所有行
    ui->tableWidget->setColumnCount(0); // 清除所有列

    // 设置列数
    int n = 2 + dfaCharSet.size(); // 默认两列：Flag 和 状态集合
    ui->tableWidget->setColumnCount(n);

    // 字符和第X列存起来对应
    map<char, int> headerCharNum;

    // 设置表头
    QStringList headerLabels;
    headerLabels << "标志" << "状态集合";
    int headerCount = 3;
    for (const auto& ch : dfaCharSet) {
        if (m1.find(ch) != m1.end()) {
            headerLabels << QString::fromStdString(trim(m1[ch]));
        }
        else {
            if (ch == '~') {
                char tt = commentSymbol[1][0];
                string res = commentSymbol[1];
                if (m1.find(tt) != m1.end()) {
                    res = trim(m1[tt]);
                }
                headerLabels << QString::fromStdString("非" + res);
            }
            else {
                headerLabels << QString(ch);
            }
        }

        headerCharNum[ch] = headerCount++;
    }
    ui->tableWidget->setHorizontalHeaderLabels(headerLabels);
    // 设置行数
    int rowCount = dfaTable.size();
    ui->tableWidget->setRowCount(rowCount);

    // 填充数据
    int row = 0;
    for (auto& dfaNode : dfaTable) {

        // Flag 列
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(dfaNode.flag)));

        // 状态集合 列
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(QString::fromStdString("{" + set2string(dfaNode.nfaStates) + "}")));

        // 状态转换 列
        int col = 2;
        for (const auto& transitionEntry : dfaNode.transitions) {
            string re = set2string(transitionEntry.second);

            // 放到指定列数据
            ui->tableWidget->setItem(row, headerCharNum[transitionEntry.first] - 1, new QTableWidgetItem(QString::fromStdString("{" + re + "}")));
            col++;
        }

        row++;
    }

    // 调整列宽
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // 显示表格
    ui->tableWidget->show();
}

/*
* @brief DFA最小化按钮
*/
void Widget::on_pushButton_5_clicked()
{
    ui->tableWidget->clearContents(); // 清除表格中的数据
    ui->tableWidget->setRowCount(0); // 清除所有行
    ui->tableWidget->setColumnCount(0); // 清除所有列

    // 设置列数
    int n = 2 + dfaCharSet.size(); // 默认两列：Flag 和 状态集合
    ui->tableWidget->setColumnCount(n);

    // 字符和第X列存起来对应
    map<char, int> headerCharNum;

    // 设置表头
    QStringList headerLabels;
    headerLabels << "标志" << "ID";
    int headerCount = 3;
    for (const auto& ch : dfaCharSet) {
        if (m1.find(ch) != m1.end()) {
            headerLabels << QString::fromStdString(trim(m1[ch]));
        }
        else {
            if (ch == '~') {
                char tt = commentSymbol[1][0];
                string res = commentSymbol[1];
                if (m1.find(tt) != m1.end()) {
                    res = trim(m1[tt]);
                }
                headerLabels << QString::fromStdString("非" + res);
            }
            else {
                headerLabels << QString(ch);
            }
        }

        headerCharNum[ch] = headerCount++;
    }
    ui->tableWidget->setHorizontalHeaderLabels(headerLabels);

    // 设置行数
    int rowCount = dfaMinTable.size();
    ui->tableWidget->setRowCount(rowCount);

    // 填充数据
    int row = 0;
    for (auto& dfaNode : dfaMinTable) {

        // Flag 列
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(dfaNode.flag)));

        // 状态集合 列
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(QString::number(dfaNode.id)));

        // 状态转换 列
        int col = 2;
        for (const auto& transitionEntry : dfaNode.transitions) {
            // 放到指定列数据
            ui->tableWidget->setItem(row, headerCharNum[transitionEntry.first] - 1, new QTableWidgetItem(transitionEntry.second == -1 ? QString::fromStdString("") : QString::number(transitionEntry.second)));
            col++;
        }

        row++;
    }

    // 调整列宽
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // 显示表格
    ui->tableWidget->show();
}

/*
* @brief LEX生成按钮
*/
void Widget::on_pushButton_2_clicked()
{
    // 只生成代码，不编译不运行
    QString srcFilePath;
    
    // 根据选择的语言类型设置不同的提示
    int langIndex = ui->comboBox_lang->currentIndex();
    QString langName = (langIndex == 0) ? "TINY" : "Mini-C";
    QString sampleExt = (langIndex == 0) ? ".tny" : ".mc";
    
    QString t_filePath = QFileDialog::getExistingDirectory(this,
        QString::fromUtf8("选择词法分析程序生成路径（") + langName + QString::fromUtf8("语言）"),
        QDir::currentPath());
    if(t_filePath.isEmpty())
        return;
    else
        srcFilePath = t_filePath;

    qDebug() << "生成" << langName << "词法分析程序...";
    
    // 根据语言类型生成不同的词法分析器
    QString res;
    if (langIndex == 0) {
        res = generateLexer(srcFilePath);  // TINY
    } else {
        res = generateMiniCLexer(srcFilePath);  // Mini-C
    }
    qDebug() << "词法分析程序生成完成...";

    /*==========文件处理=================*/
    QString cFilePath = srcFilePath + "/lexer.c";
    QFile tgtFile(cFilePath);
    if(!tgtFile.open(QIODevice::ReadWrite|QIODevice::Text|QIODevice::Truncate))
    {
        QMessageBox::warning(NULL, QString::fromUtf8("文件"), QString::fromUtf8("文件打开/写入失败"));
        return;
    }
    QTextStream outputFile(&tgtFile);
    outputFile << res;
    tgtFile.close();

    // 保存路径和语言类型供后续编译和测试使用
    m_lexerPath = srcFilePath;

    ui->tableWidget->hide();
    ui->plainTextEdit->show();
    ui->plainTextEdit->setPlainText(res);
    ui->plainTextEdit->appendPlainText("\n\n[代码生成成功] " + langName + QString::fromUtf8("词法分析器"));
    ui->plainTextEdit->appendPlainText(QString::fromUtf8("保存至: ") + cFilePath);

    // 检查示例文件是否存在
    QString sampleFile = srcFilePath + "/sample" + sampleExt;
    if (!QFile::exists(sampleFile)) {
        ui->plainTextEdit->appendPlainText(QString::fromUtf8("[提示] 请将") + langName + 
            QString::fromUtf8("源程序(如 sample") + sampleExt + QString::fromUtf8(")放入目标目录后点击[测试]按钮。"));
    }
}

void Widget::on_pushButton_10_clicked()
{
    // 编译
    if (m_lexerPath.isEmpty()) {
        QMessageBox::warning(this, QString::fromUtf8("提示"), QString::fromUtf8("请先点击[生成代码]生成词法分析器！"));
        return;
    }

    QString cFilePath = m_lexerPath + "/lexer.c";
    if (!QFile::exists(cFilePath)) {
        QMessageBox::warning(this, "错误", "未找到 lexer.c 文件，请先生成代码！");
        return;
    }

    m_exePath = m_lexerPath + "/lexer";
    #ifdef Q_OS_WIN
    m_exePath += ".exe";
    #endif

    QProcess compileProcess;
    QStringList compileArgs;
    compileArgs << cFilePath << "-o" << m_exePath;

    ui->tableWidget->hide();
    ui->plainTextEdit->show();
    ui->plainTextEdit->appendPlainText("\n\n[正在编译] gcc " + compileArgs.join(" "));

    compileProcess.start("gcc", compileArgs);
    if (!compileProcess.waitForFinished()) {
        ui->plainTextEdit->appendPlainText("[编译失败] 无法启动 gcc，请确保已安装 gcc 并添加到环境变量。");
        return;
    }

    if (compileProcess.exitCode() != 0) {
        ui->plainTextEdit->appendPlainText("[编译错误]:\n" + compileProcess.readAllStandardError());
        return;
    }

    ui->plainTextEdit->appendPlainText("[编译成功] 可执行文件: " + m_exePath);
}

void Widget::on_pushButton_11_clicked()
{
    // 测试（运行词法分析器）- 选择源文件进行测试
    if (m_lexerPath.isEmpty()) {
        QMessageBox::warning(this, QString::fromUtf8("提示"), QString::fromUtf8("请先点击[生成代码]生成词法分析器！"));
        return;
    }

    QString exePath = m_lexerPath + "/lexer";
    #ifdef Q_OS_WIN
    exePath += ".exe";
    #endif

    if (!QFile::exists(exePath)) {
        QMessageBox::warning(this, QString::fromUtf8("提示"), QString::fromUtf8("未找到可执行文件，请先点击[编译]编译词法分析器！"));
        return;
    }

    // 根据选择的语言类型设置文件过滤器
    int langIndex = ui->comboBox_lang->currentIndex();
    QString langName = (langIndex == 0) ? "TINY" : "Mini-C";
    QString fileFilter = (langIndex == 0) ? 
        QString::fromUtf8("TINY源文件 (*.tny);;所有文件 (*.*)") :
        QString::fromUtf8("Mini-C源文件 (*.mc *.c);;所有文件 (*.*)");
    QString defaultExt = (langIndex == 0) ? ".tny" : ".mc";

    // 让用户选择源文件
    QString srcFile = QFileDialog::getOpenFileName(this, 
        QString::fromUtf8("选择") + langName + QString::fromUtf8("源文件"), 
        m_lexerPath, 
        fileFilter);
    
    if (srcFile.isEmpty()) {
        return;  // 用户取消了选择
    }

    // 生成输出文件路径（替换扩展名为 .lex）
    QString outputLexPath = srcFile;
    if (outputLexPath.endsWith(".tny", Qt::CaseInsensitive) || 
        outputLexPath.endsWith(".mc", Qt::CaseInsensitive) ||
        outputLexPath.endsWith(".c", Qt::CaseInsensitive)) {
        int dotPos = outputLexPath.lastIndexOf('.');
        outputLexPath = outputLexPath.left(dotPos) + ".lex";
    } else {
        outputLexPath += ".lex";
    }

    ui->tableWidget->hide();
    ui->plainTextEdit->show();
    ui->plainTextEdit->appendPlainText("\n\n[正在运行" + langName + QString::fromUtf8("词法分析器...]"));
    ui->plainTextEdit->appendPlainText(QString::fromUtf8("输入文件: ") + srcFile);
    ui->plainTextEdit->appendPlainText(QString::fromUtf8("输出文件: ") + outputLexPath);

    QProcess runProcess;
    runProcess.setWorkingDirectory(m_lexerPath);
    
    // 使用命令行参数运行: ./lexer input.xxx output.lex
    QStringList args;
    args << srcFile << outputLexPath;
    
    runProcess.start(exePath, args);
    if (!runProcess.waitForFinished()) {
         ui->plainTextEdit->appendPlainText(QString::fromUtf8("[运行失败]"));
         return;
    }

    // 显示程序输出
    QString stdOutput = runProcess.readAllStandardOutput();
    QString stdError = runProcess.readAllStandardError();
    
    if (!stdError.isEmpty()) {
        ui->plainTextEdit->appendPlainText(QString::fromUtf8("\n[错误输出]:\n") + stdError);
    }

    ui->plainTextEdit->appendPlainText(QString::fromUtf8("\n[运行成功]\n"));
    ui->plainTextEdit->appendPlainText(QString::fromUtf8("=== ") + langName + QString::fromUtf8(" 词法分析结果 ===\n"));

    // 读取生成的 .lex 文件
    QFile lexFile(outputLexPath);
    if (lexFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&lexFile);
        ui->plainTextEdit->appendPlainText(in.readAll());
        lexFile.close();
    } else {
        ui->plainTextEdit->appendPlainText(QString::fromUtf8("[错误] 无法读取 ") + outputLexPath);
    }
}

/*
* @brief 打开文本文件 (Qt6 修复版)
* 移除了 std::ifstream 和 QTextCodec，改用 QFile 和 QTextStream
*/
void Widget::on_pushButton_6_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("选择文件"), QDir::homePath(), tr("文本文件 (*.txt);;所有文件 (*.*)"));

    if (!filePath.isEmpty())
    {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::critical(this, "错误信息", "导入错误！无法打开文件，请检查路径和文件是否被占用！");
            cerr << "Error opening file." << endl;
            return;
        }

        QTextStream in(&file);
        // Qt6 默认使用UTF-8，通常也能很好地自动检测系统编码
        // 如果必须读取GB2312且文件乱码，可能需要使用QStringConverter或转换文件编码
        QString fileContents = in.readAll();

        ui->plainTextEdit_2->setPlainText(fileContents);
        file.close();
    }
}

void Widget::on_pushButton_7_clicked()
{
    // 保存结果到文本文件
    QString saveFilePath = QFileDialog::getSaveFileName(this, tr("保存结果文件"), QDir::homePath(), tr("文本文件 (*.txt)"));
    if (!saveFilePath.isEmpty() && !ui->plainTextEdit_2->toPlainText().isEmpty()) {
        QFile outputFile(saveFilePath);
        if (outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&outputFile);
            stream << ui->plainTextEdit_2->toPlainText();
            outputFile.close();
            QMessageBox::about(this, "提示", "导出成功！");
        }
    }
    else if (ui->plainTextEdit_2->toPlainText().isEmpty())
    {
        QMessageBox::warning(this, tr("提示"), tr("输入框为空，请重试！"));
    }
}

/*
* @brief 打开LEX文件 (Qt6 修复版)
* 移除了 std::ifstream 和 QTextCodec，改用 QFile 和 QTextStream
*/
void Widget::on_pushButton_8_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("选择文件"), QDir::homePath(), tr("LEX文件 (*.lex)"));

    if (!filePath.isEmpty())
    {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::critical(this, "错误信息", "导入错误！无法打开文件，请检查路径和文件是否被占用！");
            cerr << "Error opening file." << endl;
            return;
        }

        QTextStream in(&file);
        QString fileContents = in.readAll();

        ui->tableWidget->hide();
        ui->plainTextEdit->show();
        ui->plainTextEdit->setPlainText(fileContents);
        file.close();
    }
}
