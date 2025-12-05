/****************************************************
 * @Copyright © 2023-2024 LDL. All rights reserved.
 *
 * @FileName: widget.cpp
 * @Brief: 主程序
 * @Module Function:
 *
 * @Current Version: 1.0
 * @Author: Lidaliang
 * @Modifier: Lidaliang
 * @Finished Date: 2024/4/21
 *
 * @Version History: 1.0 初始化
 *
 ****************************************************/
#include "widget.h"
#include "ui_widget.h"
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QFileDialog>
#include <QTextCodec>
#include <QMessageBox>
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
#pragma execution_character_set("utf-8")
using namespace std;

Widget::Widget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
}

Widget::~Widget()
{
    delete ui;
}

// 非终结符数组
vector<string> bigAlpha;
// 终结符数组
vector<string> smallAlpha;

// 全局文法变量
string grammarStr;

// 结构化后的文法map
unordered_map<string, set<string>> grammarMap;

// 文法unit（用于LR0）
struct grammarUnit
{
    int gid;
    string left;
    string right;
    grammarUnit(string l, string r)
    {
        left = l;
        right = r;
    }
};

// 文法数组（用于LR0）
deque<grammarUnit> grammarDeque;

// LR0结果提示字符串
QString LR0Result;

// 文法查找下标
map<pair<string, string>, int> grammarToInt;

/*************  公用函数 ****************/
/*
* @brief 判断是否为非终结符
*/
bool isBigAlpha(string c)
{
    for (const auto& symbol : bigAlpha)
    {
        if (symbol == c)
        {
            return true;
        }
    }
    return false;
}

/*
* @brief 判断是否为终结符
*/
bool isSmallAlpha(string c)
{
    for (const auto& symbol : smallAlpha)
    {
        if (symbol == c)
        {
            return true;
        }
    }
    return false;
}

void reset();

// 开始符号
string startSymbol;

// 增广后开始符号
string trueStartSymbol;

/************* 文法初始化处理 ****************/

/*
* @brief 文法初始化
* 输入规则：
* 用@表示空串
* 第一行输入非终结符，用|分隔
* 第二行输入终结符，用|分隔
* 剩余行数均为文法，文法输入时，单词之间用空格分隔，
* 默认左边出现的第一个字符串为文法的开始符号
* 同时，文法中含有或(|)，请分开两条输入
*/
void handleGrammar()
{
    vector<string> lines;
    istringstream iss(grammarStr);
    string line;

    // 防止中间有换行符
    while (getline(iss, line))
    {
        if (!line.empty())
        {
            lines.push_back(line);
        }
    }

    // 非终结符
    QString line1 = QString::fromStdString(lines[0]);
    QStringList tokens1 = line1.split("|");

    for (const QString& token : tokens1) {
        // 去除空格
        QString trimmedToken = token.trimmed();
        if (!trimmedToken.isEmpty()) {
            bigAlpha.push_back(trimmedToken.toStdString());
        }
    }

    // 终结符
    QString line2 = QString::fromStdString(lines[1]);
    QStringList tokens2 = line2.split("|");

    for (const QString& token : tokens2) {
        // 去除空格
        QString trimmedToken = token.trimmed();
        if (!trimmedToken.isEmpty()) {
            smallAlpha.push_back(trimmedToken.toStdString());
        }
    }

    for (int i = 2; i < lines.size(); i++)
    {
        string rule = lines[i];
        istringstream ruleStream(rule);
        string nonTerminal;
        ruleStream >> nonTerminal;  // 读取非终结符

        // 验证非终结符的格式
        if (!isBigAlpha(nonTerminal))
        {
            QMessageBox::critical(nullptr, "Error", "文法开头必须是非终结符!");
            continue;
        }

        // 跳过箭头符号 "->"
        string arrow;
        ruleStream >> arrow;

        string rightHandSide;
        getline(ruleStream, rightHandSide);

        // 去除开头的空格
        rightHandSide = rightHandSide.substr(1);

        // 如果是第一条规则，则认为是开始符号
        if (grammarMap.empty())
        {
            startSymbol = nonTerminal;
            trueStartSymbol = startSymbol;
        }

        // 将文法结构化
        grammarMap[nonTerminal].insert(QString::fromStdString(rightHandSide).trimmed().toStdString());

        // 为LR0做准备
        grammarDeque.push_back(grammarUnit(nonTerminal, rightHandSide));
    }

    // 增广处理
    if (grammarMap[startSymbol].size() > 1)
    {
        // 如果开始符号多于2个，说明需要增广，为了避免出现字母重复，采用^作为增广后的字母，后期输出特殊处理
        grammarDeque.push_front(grammarUnit("zengguang", startSymbol));
        LR0Result += QString::fromStdString("进行了增广处理\n");
        trueStartSymbol = "zengguang";
    }

    // 开始编号
    int gid = 0;
    for (auto& g : grammarDeque)
    {
        g.gid = gid++;
        LR0Result += QString::number(g.gid) + QString::fromStdString(":")
            + QString::fromStdString(g.left == "zengguang" ? "E\'" : g.left) + QString::fromStdString("->")
            + QString::fromStdString(g.right) + "\n";
        // 存入map中
        grammarToInt[make_pair(g.left, g.right)] = g.gid;
    }

}

/************* First集合求解 ****************/


// First集合单元
struct firstUnit
{
    set<string> s;
    bool isEpsilon = false;
};

// 非终结符的First集合
map<string, firstUnit> firstSets;

/*
* @brief 计算First集合
*/
bool calculateFirstSets()
{
    bool flag = false;
    for (auto& grammar : grammarMap)
    {
        string nonTerminal = grammar.first;
        // 保存当前First集合的大小，用于检查是否有变化
        size_t originalSize = firstSets[nonTerminal].s.size();
        bool originalE = firstSets[nonTerminal].isEpsilon;
        for (auto& g : grammar.second)
        {
            QStringList gList = QString::fromStdString(g).split(" ");
            int k = 0;
            while (k <= gList.size() - 1)
            {
                string t = gList[k].toStdString();
                set<string> first_k;
                if (t == "@")
                {
                    k++;
                    continue;
                }
                else if (isSmallAlpha(t))
                {
                    first_k.insert(t);
                }
                else
                {
                    first_k = firstSets[t].s;
                }
                firstSets[nonTerminal].s.insert(first_k.begin(), first_k.end());
                // 如果是终结符或者没有空串在非终结符中，直接跳出
                if (isSmallAlpha(t) || !firstSets[t].isEpsilon)
                {
                    break;
                }
                k++;
            }
            if (k == g.size())
            {
                firstSets[nonTerminal].isEpsilon = true;
            }
        }
        // 看原始大小和是否变化epsilon，如果变化说明还得重新再来一次
        if (originalSize != firstSets[nonTerminal].s.size() || originalE != firstSets[nonTerminal].isEpsilon)
        {
            flag = true;
        }
    }
    return flag;
}

/*
* @brief 求First集合入口
*/
void getFirstSets()
{
    // 不停迭代，直到First集合不再变化
    bool flag = false;
    do
    {
        flag = calculateFirstSets();
    } while (flag);
}


/************* Follow集合求解 ****************/
// Follow集合单元
struct followUnit
{
    set<string> s;
};

// 非终结符的Follow集合
map<string, followUnit> followSets;

/*
* @brief 添加Follow集合
*/
void addToFollow(string nonTerminal, const set<string>& elements)
{
    followSets[nonTerminal].s.insert(elements.begin(), elements.end());
}

/*
* @brief 计算Follow集合
*/
bool calculateFollowSets()
{
    bool flag = false;
    for (auto& grammar : grammarMap)
    {
        string nonTerminal = grammar.first;

        for (auto& g : grammar.second)
        {
            QStringList gList = QString::fromStdString(g).split(" ");
            for (int i = 0; i < gList.size(); ++i)
            {
                string t = gList[i].toStdString();
                if (isSmallAlpha(t) || t == "@")
                {
                    continue;  // 跳过终结符
                }

                set<string> follow_k;
                size_t originalSize = followSets[t].s.size();

                if (i == gList.size() - 1)
                {
                    // Case A: A -> αB, add Follow(A) to Follow(B)
                    follow_k.insert(followSets[nonTerminal].s.begin(), followSets[nonTerminal].s.end());
                }
                else
                {
                    // Case B: A -> αBβ
                    int j = i + 1;
                    while (j < gList.size())
                    {
                        string t2 = gList[j].toStdString();
                        if (isSmallAlpha(t2))
                        {   // 终结符直接加入并跳出
                            follow_k.insert(t2);
                            break;
                        }
                        else
                        {   // 非终结符加入first集合
                            set<string> first_beta = firstSets[t2].s;
                            follow_k.insert(first_beta.begin(), first_beta.end());

                            // 如果没有空串在first集合中，停止。
                            if (!firstSets[t2].isEpsilon)
                            {
                                break;
                            }

                            ++j;
                        }

                    }

                    // If β is ε or β is all nullable, add Follow(A) to Follow(B)
                    if (j == gList.size())
                    {
                        follow_k.insert(followSets[nonTerminal].s.begin(), followSets[nonTerminal].s.end());
                    }
                }

                addToFollow(t, follow_k);
                // 检查是否变化
                if (originalSize != followSets[t].s.size())
                {
                    flag = true;
                }
            }
        }


    }

    return flag;
}

/*
* @brief 求Follow集合入口
*/
void getFollowSets()
{
    // 开始符号加入$
    addToFollow(startSymbol, { "$" });

    // 不停迭代，直到Follow集合不再变化
    bool flag = false;
    do
    {
        flag = calculateFollowSets();
    } while (flag);
}

/************* LR0 DFA表生成 ****************/

// 状态编号
int scnt = 0;
// 项目编号
int ccnt = 0;

// DFA表每一项项目的结构
struct dfaCell
{
    int cellid; // 这一项的编号，便于后续判断状态相同
    int gid; // 文法编号
    int index = 0; // .在第几位，如i=3, xxx.x，i=0, .xxxx, i=4, xxxx
};

// 用于通过编号快速找到对应结构
vector<dfaCell> dfaCellVector;

struct nextStateUnit
{
    string c; // 通过什么字符进入这个状态
    int sid; // 下一个状态id是什么
};

// DFA表状态
struct dfaState
{
    int sid; // 状态id
    vector<int> originV;    // 未闭包前的cell
    vector<int> cellV;  // 存储这个状态的cellid
    bool isEnd = false; // 是否为规约状态
    bool isSpecial = false; //是否是规约移进冲突
    vector<nextStateUnit> nextStateVector; // 下一个状态集合
    set<string> right_VNs; // 判断是否已经处理过这个非终结符
};

// 用于通过编号快速找到对应结构
vector<dfaState> dfaStateVector;

// 非终结符集合
set<string> VN;
// 终结符集合
set<string> VT;

/*
* @brief 判断是不是新结构
*/
int isNewCell(int gid, int index)
{
    for (const dfaCell& cell : dfaCellVector)
    {
        // 检查dfaCellVector中是否存在相同的gid和index的dfaCell
        if (cell.gid == gid && cell.index == index)
        {
            return cell.cellid; // 不是新结构
        }
    }
    return -1; // 是新结构
}

/*
* @brief 判断是不是新状态
*/
int isNewState(const vector<int>& cellIds)
{
    for (const dfaState& state : dfaStateVector)
    {
        // 检查状态中的originV是否相同
        if (state.originV.size() == cellIds.size() &&
            equal(state.originV.begin(), state.originV.end(), cellIds.begin()))
        {
            return state.sid; // 不是新状态
        }
    }

    return -1; // 是新状态
}

// DFS标记数组
set<int> visitedStates;

/*
* @brief 创建LR0初始状态
*/
void createFirstState()
{
    // 由于增广，一定会只有一个入口
    dfaState zero = dfaState();
    zero.sid = scnt++; // 给他一个id
    dfaStateVector.push_back(zero); // 放入数组中

    // 添加初始的LR0项，即E' -> .S
    dfaCell startCell;
    startCell.gid = 0; // 这里假设增广文法的编号为0
    startCell.index = 0;
    startCell.cellid = ccnt++;

    dfaCellVector.push_back(startCell);

    // 把初始LR0项放入初始状态
    dfaStateVector[0].cellV.push_back(startCell.cellid);
    dfaStateVector[0].originV.push_back(startCell.cellid);
}

/*
* @brief 生成LR0状态
*/
void generateLR0State(int stateId)
{
    // DFS,如果走过就不走了
    if (visitedStates.count(stateId) > 0) {
        return;
    }

    // 标记走过了
    visitedStates.insert(stateId);

    qDebug() << stateId;

    // 求闭包
    for (int i = 0; i < dfaStateVector[stateId].cellV.size(); ++i)
    {
        dfaCell& currentCell = dfaCellVector[dfaStateVector[stateId].cellV[i]];

        qDebug() << QString::fromStdString(grammarDeque[currentCell.gid].left) << QString::fromStdString("->") << QString::fromStdString(grammarDeque[currentCell.gid].right);

        qDebug() << "current index:" << currentCell.index;

        QStringList rightList = QString::fromStdString(grammarDeque[currentCell.gid].right).split(" ");

        // 如果点号在产生式末尾或者空串，则跳过（LR0不需要结束）
        if (currentCell.index == rightList.size() || grammarDeque[currentCell.gid].right == "@")
        {
            dfaStateVector[stateId].isEnd = true;
            continue;
        }

        string nextSymbol = rightList[currentCell.index].toStdString();


        // 如果nextSymbol是非终结符，则将新项添加到状态中
        if (isBigAlpha(nextSymbol) && dfaStateVector[stateId].right_VNs.find(nextSymbol) == dfaStateVector[stateId].right_VNs.end())
        {
            dfaStateVector[stateId].right_VNs.insert(nextSymbol);
            for (auto& grammar : grammarMap[nextSymbol])
            {
                // 获取通过nextSymbol转移的新LR0项
                dfaCell nextCell = dfaCell();
                nextCell.gid = grammarToInt[make_pair(nextSymbol, grammar)];
                nextCell.index = 0;
                int nextcellid = isNewCell(nextCell.gid, nextCell.index);
                if (nextcellid == -1)
                {
                    nextCell.cellid = ccnt++;
                    dfaCellVector.push_back(nextCell);
                    dfaStateVector[stateId].cellV.push_back(nextCell.cellid);
                }
                else dfaStateVector[stateId].cellV.push_back(nextcellid);
            }

        }
    }

    // 暂存新状态
    map<string, dfaState> tempSave;
    // 生成新状态，但还不能直接存到dfaStateVector中，我们要校验他是否和之前的状态一样
    for (int i = 0; i < dfaStateVector[stateId].cellV.size(); ++i)
    {
        dfaCell& currentCell = dfaCellVector[dfaStateVector[stateId].cellV[i]];

        QStringList rightList = QString::fromStdString(grammarDeque[currentCell.gid].right).split(" ");

        // 如果点号在产生式末尾，则跳过（LR0不需要结束）
        if (currentCell.index == rightList.size() || grammarDeque[currentCell.gid].right == "@")
        {
            continue;
        }

        // 下一个字符
        string nextSymbol = rightList[currentCell.index].toStdString();

        // 创建下一个状态（临时的）
        dfaState& nextState = tempSave[nextSymbol];
        dfaCell nextStateCell = dfaCell();
        nextStateCell.gid = currentCell.gid;
        nextStateCell.index = currentCell.index + 1;

        // 看看里面的项目是否有重复的，如果重复拿之前的就好，不重复生成
        int nextStateCellid = isNewCell(nextStateCell.gid, nextStateCell.index);
        if (nextStateCellid == -1)
        {
            nextStateCell.cellid = ccnt++;
            dfaCellVector.push_back(nextStateCell);
        }
        else nextStateCell.cellid = nextStateCellid;
        nextState.cellV.push_back(nextStateCell.cellid);
        nextState.originV.push_back(nextStateCell.cellid);

        // 收集一下，方便后面画表
        if (isBigAlpha(nextSymbol))
        {
            VN.insert(nextSymbol);
        }
        else if (isSmallAlpha(nextSymbol))
        {
            VT.insert(nextSymbol);
        }
    }

    // 校验状态是否有重复的
    for (auto& t : tempSave)
    {
        dfaState nextState = dfaState();
        int newStateId = isNewState(t.second.originV);
        // 不重复就新开一个状态
        if (newStateId == -1)
        {
            nextState.sid = scnt++;
            nextState.cellV = t.second.cellV;
            nextState.originV = t.second.originV;
            dfaStateVector.push_back(nextState);
        }
        else nextState.sid = newStateId;
        // 存入现在这个状态的nextStateVector
        nextStateUnit n = nextStateUnit();
        n.sid = nextState.sid;
        n.c = t.first;
        dfaStateVector[stateId].nextStateVector.push_back(n);
    }

    // 对每个下一个状态进行递归
    int nsize = dfaStateVector[stateId].nextStateVector.size();
    for (int i = 0; i < nsize; i++)
    {
        auto& nextunit = dfaStateVector[stateId].nextStateVector[i];
        generateLR0State(nextunit.sid);
    }
}

/*
* @brief LR0生成入口
*/
void getLR0()
{
    visitedStates.clear();

    // 首先生成第一个状态
    createFirstState();

    // 递归生成其他状态
    generateLR0State(0);
}

/*
* @brief 拼接字符串，获取状态内的文法
*/
string getStateGrammar(const dfaState& d)
{
    string result = "";
    for (auto cell : d.cellV)
    {
        const dfaCell& dfaCell = dfaCellVector[cell];
        // 拿到文法
        int gid = dfaCell.gid;
        grammarUnit g = grammarDeque[gid];
        // 拿到位置
        int index = dfaCell.index;
        // 拼接结果
        string r = "";
        r += g.left == "zengguang" ? "E\'->" : g.left + "->";
        string right = g.right == "@" ? "" : g.right;
        string rightResult = "";
        QStringList rightList = QString::fromStdString(right).split(" ");
        for (int i = 0; i <= rightList.size(); i++) {
            if (i == index) {
                rightResult += ".";
            }
            if (i == rightList.size()) break;
            rightResult += rightList[i].toStdString();
        }
        //right.insert(i, 1, '.');
        r += rightResult;
        result += r + " ";
    }
    return result;
}


/******************** SLR1分析 ***************************/
/*
* @brief 检查移进规约冲突
*/
bool SLR1Fun1()
{
    bool flag = false;
    for (dfaState& state : dfaStateVector)
    {
        // 规约项目的左边集合
        set<string> a;
        // 终结符
        set<string> rVT;
        // 不是规约状态不考虑
        if (!state.isEnd) continue;
        // 规约状态
        for (int cellid : state.cellV)
        {
            // 拿到这个cell
            const dfaCell& cell = dfaCellVector[cellid];
            // 获取文法
            const grammarUnit gm = grammarDeque[cell.gid];

            QStringList rightList = QString::fromStdString(grammarDeque[cell.gid].right).split(" ");
            // 判断是不是规约项目
            if (cell.index == rightList.size() || gm.right == "@")
            {
                a.insert(gm.left);
            }
            // 判断是不是终结符
            else
            {
                if (isSmallAlpha(rightList[cell.index].toStdString()))
                {
                    rVT.insert(rightList[cell.index].toStdString());
                }
            }
        }
        for (string c : a)
        {
            for (string v : rVT)
            {
                if (followSets[c].s.find(v) != followSets[c].s.end())
                {
                    flag = true;
                    state.isSpecial = true;
                }

            }
        }
    }
    return flag;
}

/*
* @brief 检查规约规约冲突
*/
bool SLR1Fun2()
{
    for (const auto& state : dfaStateVector)
    {
        // 规约项目的左边集合
        set<string> a;
        // 不是规约状态不考虑
        if (!state.isEnd) continue;

        // 规约状态
        for (int cellid : state.cellV)
        {
            // 拿到这个cell
            const dfaCell& cell = dfaCellVector[cellid];
            // 获取文法
            const grammarUnit gm = grammarDeque[cell.gid];

            QStringList rightList = QString::fromStdString(grammarDeque[cell.gid].right).split(" ");
            // 判断是不是规约项目
            if (cell.index == rightList.size() || gm.right == "@")
            {
                a.insert(gm.left);
            }
        }

        for (string c1 : a)
        {
            for (string c2 : a)
            {
                if (c1 != c2)
                {
                    // 判断followSets[c1]和followSets[c2]是否有交集
                    set<string> followSetC1 = followSets[c1].s;
                    set<string> followSetC2 = followSets[c2].s;
                    set<string> intersection;

                    // 利用STL算法求交集
                    set_intersection(
                        followSetC1.begin(), followSetC1.end(),
                        followSetC2.begin(), followSetC2.end(),
                        inserter(intersection, intersection.begin())
                    );

                    // 如果交集非空，说明存在规约-规约冲突
                    if (!intersection.empty())
                    {
                        return true;
                    }
                }
            }
        }
    }


    return false;
}


/*
* @brief 检查SLR1分析入口
*/
int SLR1Analyse()
{
    // 开始符号添加follow集合
    followSets["zengguang"].s.insert("$");

    bool flag1 = SLR1Fun1();
    bool flag2 = SLR1Fun2();
    if (flag1 && flag2)
    {
        return 3;
    }
    else if (flag1)
    {
        return 1;
    }
    else if (flag2)
    {
        return 2;
    }
    // 没有冲突，是SLR(1)文法
    return 0;
}

struct SLRUnit
{
    map<string, string> m;
};

vector<SLRUnit> SLRVector;

/*
* @brief 获取SLR1分析表
*/
int getSLR1Table()
{
    // SLR1分析错误就直接停止
    int r = SLR1Analyse();
    if (r != 0 && r != 1) return r;
    // 如果分析正确，通过LR0构造SLR1分析表（必须先调用getLR0）
    for (const dfaState& ds : dfaStateVector)
    {
        SLRUnit slrunit = SLRUnit();
        // 如果是归约，得做特殊处理
        if (ds.isEnd)
        {
            if (!ds.isSpecial) // 对于移进规约冲突的项，只做移进不做规约
            {
                // 规约项目的非终结符
                string gl;
                string gr;
                // 规约状态
                for (int cellid : ds.cellV)
                {
                    // 拿到这个cell
                    const dfaCell& cell = dfaCellVector[cellid];
                    // 获取文法
                    const grammarUnit gm = grammarDeque[cell.gid];

                    QStringList rightList = QString::fromStdString(grammarDeque[cell.gid].right).split(" ");

                    // 判断是不是规约项目
                    if (cell.index == rightList.size() || gm.right == "@")
                    {
                        gl = gm.left;
                        gr = gm.right;
                        break;  // 前面的SLR1校验保证了只有一个归约项目
                    }
                }
                // 得到这个非终结符Follow集合
                set<string> follow = followSets[gl].s;
                // follow集合每个元素都能归约
                for (string ch : follow)
                {
                    if (gl == trueStartSymbol) slrunit.m[ch] = "ACCEPT";
                    else slrunit.m[ch] = "r(" + gl + "->" + gr + ")";
                }
            }
            // 对于下一个节点（可能会存在）
            for (const auto& next : ds.nextStateVector)
            {
                string ch = next.c;
                int sid = next.sid; //  下一个状态id

                // 获取下一个状态具体信息
                dfaState d = dfaStateVector[sid];
                if (isBigAlpha(ch))
                {
                    slrunit.m[ch] = to_string(sid);
                }
                else
                {
                    slrunit.m[ch] = "s" + to_string(sid);
                }
            }
        }
        else
        {
            for (const auto& next : ds.nextStateVector)
            {
                string ch = next.c;
                int sid = next.sid; //  下一个状态id
                // 获取下一个状态具体信息
                dfaState d = dfaStateVector[sid];
                if (isBigAlpha(ch))
                {
                    slrunit.m[ch] = to_string(sid);
                }
                else
                {
                    slrunit.m[ch] = "s" + to_string(sid);
                }
            }
        }
        SLRVector.push_back(slrunit);
    }
    return r;
}

/*
* @brief 将 SLRVector 转换为自定义格式的字符串
*/
string SLRVectorToString(const vector<SLRUnit>& vec)
{
    ostringstream oss; // 创建一个输出流

    for (const auto& unit : vec)
    {
        oss << "SLRUnit\n{\n"; // 每个 SLRUnit 开始的标记

        for (const auto& pair : unit.m)
        {
            oss << "    " << "Key: " << pair.first << "\n"; // 输出键
            oss << "    " << "Value: " << pair.second << "\n"; // 输出值
        }

        oss << "}\n"; // SLRUnit 结束的标记
    }

    return oss.str(); // 返回输出流中的字符串
}

/*
* @brief 将自定义格式的字符串解析成 SLRVector
*/
vector<SLRUnit> StringToSLRVector(const string& str)
{
    vector<SLRUnit> vec;
    SLRUnit unit;

    istringstream iss(str); // 创建一个输入流
    string line;

    while (getline(iss, line))
    {
        if (line == "SLRUnit")
        {
            unit = SLRUnit(); // 创建一个新的 SLRUnit
        }
        else if (line == "{")
        {
            // 忽略
        }
        else if (line == "}")
        {
            vec.push_back(unit); // 将完成的 SLRUnit 添加到 vector 中
        }
        else if (line.size() > 0)
        {
            // 解析键值对
            size_t pos = line.find(": ");
            if (pos != string::npos)
            {
                string key = line.substr(pos + 2); // 获取键的内容
                getline(iss, line); // 读取下一行，这一行应该是值
                pos = line.find(": ");
                if (pos != string::npos)
                {
                    string value = line.substr(pos + 2); // 获取值的内容
                    unit.m[key] = value; // 添加到当前 SLRUnit 中
                }
            }
        }
    }

    return vec;
}

// 标记生成语义函数是否出错
int funCodeError = 0;
vector<int> funNumber;

/*
* @brief 生成语义函数
*/
QString generateFunCode(QString funQStr) {
    QString funCode;

    // 先进行分行处理
    vector<string> lines;
    istringstream iss(funQStr.toStdString());
    string line;

    if (grammarDeque[0].left == "zengguang") {
        lines.push_back("zengguang");
    }

    // 防止中间有换行符
    while (getline(iss, line))
    {
        if (!line.empty())
        {
            lines.push_back(line);
        }
    }

    if (lines.size() != grammarDeque.size()) {
        funCodeError = 1;   // 如果行数不一致代表有问题
        return "";
    }

    for (int i = 0; i < lines.size(); i++) {

        map<int, int> treeNodeMap;
        int count = 0;
        if (lines[i] == "zengguang") continue;

        QStringList numList = QString::fromStdString(lines[i]).split(" ");
        QStringList stringList = QString::fromStdString(grammarDeque[i].right).split(" ");

        for (int j = 0; j < stringList.size(); j++) {
            int index = stoi(numList[j].toStdString());
            if (index == -1) continue;
            else {
                treeNodeMap[index] = j;
                count++;
            }
        }

        funCode += "// ";
        funCode += QString::fromStdString(grammarDeque[i].left + "->" + grammarDeque[i].right);
        funCode += "\n";
        funCode += "string fun";
        funCode += QString::number(i);
        funNumber.push_back(i);
        funCode += "() {\n";

        for (int j = stringList.size() - 1; j >= 0; j--) {
            if (isBigAlpha(stringList[j].toStdString())) {  // 非终结符
                funCode += "\tBTreeNode* newNode";
                funCode += QString::number(j);
                funCode += R"( = treeStack.top();
    strStack.pop();
    stateStack.pop();
    treeStack.pop();)";
                funCode += "\n";
            }
            else if (isSmallAlpha(stringList[j].toStdString())) {   // 终结符
                funCode += "\tstring symbol";
                funCode += QString::number(j);
                funCode += R"( = strStack.top().value;
    strStack.pop();
    stateStack.pop();)";
                funCode += "\n";
                funCode += "\tBTreeNode* newNode";
                funCode += QString::number(j);
                funCode += " = new BTreeNode(\"";
                funCode += QString::fromStdString(stringList[j].toStdString());
                funCode += "\", symbol";
                funCode += QString::number(j);
                funCode += ");\n";
            }
            funCode += "\n";

            
        }

        // 左递归
        if (grammarDeque[i].left == stringList[0].toStdString()) {
            funCode += "\tBTreeNode* rootNode = new BTreeNode(\"-1\",\"";
            funCode += QString::fromStdString(grammarDeque[i].left);
            funCode += "\");\n ";
            for (int k = 0; k < count; k++) {
                funCode += "\trootNode";
                funCode += "->nodeList.push_back(newNode";
                funCode += QString::number(treeNodeMap[k]);
                funCode += ");\n";
            }
            funCode += "\ttreeStack.push(rootNode);\n";
        }
        else {
            for (int k = 1; k < count; k++) {
                funCode += "\tnewNode";
                funCode += QString::number(treeNodeMap[0]);
                funCode += "->nodeList.push_back(newNode";
                funCode += QString::number(treeNodeMap[k]);
                funCode += ");\n";
            }

            if (grammarDeque[i].right != "@") {
                funCode += "\ttreeStack.push(newNode";
                funCode += QString::number(treeNodeMap[0]);
                funCode += ");\n";
            }
            else
            {
                funCode += "\tBTreeNode* rootNode = new BTreeNode(\"-2\",\"";
                funCode += QString::fromStdString(grammarDeque[i].left);
                funCode += "\");\n ";
                funCode += "\ttreeStack.push(rootNode);\n";
            }
        }

        

        funCode += "\treturn \"";
        funCode += QString::fromStdString(grammarDeque[i].left);
        funCode += "\";\n";

        funCode += "}\n\n";

    }

    return funCode;
}

/*
* @brief 生成语法树代码
* @param filePath 输出目录路径
* @param lexFilePath lex文件完整路径
* @param funQStr 语义函数内容
*/
QString generateTreeCode(QString filePath, QString lexFilePath, QString funQStr) {
    QString code;

    // 头文件
    code += R"(#include <iostream>
#include <stack>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <fstream>

using namespace std;
)";

    // 生成对应函数的map
    code += "map<string, int> grammarMap = { ";
    for (int i = 0; i < grammarDeque.size(); i++) {
        if (grammarDeque[i].left == "zengguang") continue;
        code += "{\"";
        code += QString::fromStdString(grammarDeque[i].left + "->" + grammarDeque[i].right);
        code += "\" , ";
        code += QString::number(i);
        code += "},";
    }
    code.chop(1);
    code += "};\n";

    // 定义结构体和通用函数
    code += R"CODE(// 定义一个结构体来表示每一行的键值对
struct KeyValue {
    string key;
    string value;
    KeyValue() {}
    KeyValue(string _key) {
        key = _key;
    }
    KeyValue(string _key, string _value) {
        key = _key;
        value = _value;
    }
};

// TINY语言的Token映射表
map<string, map<string, string>> tinyTokenMap = {
    {"KEYWORD", {{"read", "read"}, {"write", "write"}, {"if", "if"}, {"then", "then"}, {"else", "else"}, {"end", "end"}, {"repeat", "repeat"}, {"until", "until"}}},
    {"OPERATOR", {{":", "ASSIGN"}, {"+", "PLUS"}, {"-", "MINUS"}, {"*", "MULTIPLY"}, {"/", "DIVIDE"}, {"%", "MOD"}, {"^", "POWER"}, {"<", "LT"}, {">", "RT"}, {"=", "EQ"}}},
    {"DELIMITER", {{";", "SEMI"}, {"(", "LPAN"}, {")", "RPAN"}}}
};

// MiniC语言的Token映射表
map<string, map<string, string>> minicTokenMap = {
    {"KEYWORD", {{"int", "int"}, {"float", "float"}, {"real", "real"}, {"void", "void"}, {"if", "if"}, {"else", "else"}, {"while", "while"}, {"for", "for"}, {"return", "return"}}},
    {"OPERATOR", {{"=", "ASSIGN"}, {"+", "PLUS"}, {"-", "MINUS"}, {"*", "MULTIPLY"}, {"/", "DIVIDE"}, {"%", "MOD"}, {"^", "POWER"}, {"<", "LT"}, {">", "RT"}}},
    {"DELIMITER", {{";", "SEMI"}, {"(", "LPAREN"}, {")", "RPAREN"}, {"{", "LBRACE"}, {"}", "RBRACE"}, {"[", "LBRACKET"}, {"]", "RBRACKET"}, {",", "COMMA"}}}
};

// 判断是否是TINY语言格式
bool isTinyLanguage = true;

// 转换Token类型
string convertToken(const string& tokenType, const string& tokenValue) {
    auto& tokenMap = isTinyLanguage ? tinyTokenMap : minicTokenMap;
    
    if (tokenType == "KEYWORD") {
        return tokenValue;
    } else if (tokenType == "ID") {
        return "ID";
    } else if (tokenType == "NUM" || tokenType == "NUMBER") {
        return "NUMBER";
    } else if (tokenType == "FLOAT") {
        return "FLOAT";
    } else if (tokenType == "OPERATOR") {
        // 处理多字符运算符
        if (tokenValue == ":=") return "ASSIGN";
        if (tokenValue == "<=") return "LTEQ";
        if (tokenValue == ">=") return "RTEQ";
        if (tokenValue == "<>") return "NE";
        if (tokenValue == "==") return "EQ";
        if (tokenValue == "!=") return "NE";
        if (tokenValue == "++") return "INC";
        if (tokenValue == "--") return "DEC";
        if (tokenMap["OPERATOR"].count(tokenValue)) {
            return tokenMap["OPERATOR"][tokenValue];
        }
        return tokenValue;
    } else if (tokenType == "DELIMITER") {
        if (tokenMap["DELIMITER"].count(tokenValue)) {
            return tokenMap["DELIMITER"][tokenValue];
        }
        return tokenValue;
    }
    return tokenType;
}

// 函数读取并分隔每一行的键值对 (适配词法分析器输出格式)
vector<KeyValue> readKeyValuePairs(const string& filename) {
    ifstream file(filename);
    vector<KeyValue> keyValuePairs;

    if (!file) {
        cerr << "无法打开文件: " << filename << endl;
        return keyValuePairs;
    }

    // 检测文件类型
    string firstLine;
    getline(file, firstLine);
    if (firstLine.find("Mini-C") != string::npos || firstLine.find("MiniC") != string::npos || firstLine.find("mini-c") != string::npos) {
        isTinyLanguage = false;
    } else {
        isTinyLanguage = true;
    }

    string line;
    while (getline(file, line)) {
        if (line.empty() || line.find("===") != string::npos) {
            continue;
        }

        size_t colonPos = line.find(':');
        if (colonPos == string::npos) continue;
        
        string afterColon = line.substr(colonPos + 1);
        size_t commaPos = afterColon.find(',');
        if (commaPos == string::npos) continue;
        
        string tokenType = afterColon.substr(0, commaPos);
        string tokenValue = afterColon.substr(commaPos + 1);
        
        tokenType.erase(0, tokenType.find_first_not_of(" \t"));
        tokenType.erase(tokenType.find_last_not_of(" \t") + 1);
        tokenValue.erase(0, tokenValue.find_first_not_of(" \t"));
        tokenValue.erase(tokenValue.find_last_not_of(" \t") + 1);
        
        string convertedType = convertToken(tokenType, tokenValue);
        
        keyValuePairs.push_back({ convertedType, tokenValue });
    }
    
    keyValuePairs.push_back({ "EOF", "$" });

    file.close();
    return keyValuePairs;
}

// 定义语法树节点结构
struct BTreeNode {
    string kind;
    string value;
    vector<BTreeNode*> nodeList;
    BTreeNode(string kind,string val) :kind(kind), value(val) {}
};

stack<BTreeNode*> treeStack;
stack<KeyValue> strStack;
stack<int> stateStack;

struct SLRUnit
{
    string index;
    map<string, string> m;
};

vector<SLRUnit> SLRVector;

vector<SLRUnit> StringToSLRVector(const string& str)
{
    vector<SLRUnit> vec;
    SLRUnit unit;

    istringstream iss(str);
    string line;

    int index = 0;

    while (getline(iss, line))
    {
        if (line == "SLRUnit")
        {
            unit = SLRUnit();
            unit.index = to_string(index);
        }
        else if (line == "{")
        {
        }
        else if (line == "}")
        {
            vec.push_back(unit);
            index++;
        }
        else if (line.size() > 0)
        {
            size_t pos = line.find(": ");
            if (pos != string::npos)
            {
                string key = line.substr(pos + 2);
                getline(iss, line);
                pos = line.find(": ");
                if (pos != string::npos)
                {
                    string value = line.substr(pos + 2);
                    unit.m[key] = value;
                }
            }
        }
    }

    return vec;
}

string BTreeNodeToString(BTreeNode* node, int depth = 0)
{
    ostringstream oss;
    string indent(depth * 4, ' ');

    oss << indent << "BTreeNode\n";
    oss << indent << "{\n";
    oss << indent << "    kind: " << node->kind << "\n";
    oss << indent << "    value: " << node->value << "\n";

    if (!node->nodeList.empty())
    {
        oss << indent << "    nodeList:\n";
        for (const auto& child : node->nodeList)
        {
            oss << BTreeNodeToString(child, depth + 1);
        }
    }

    oss << indent << "}\n";

    return oss.str();
}
)CODE";

    // 生成语义函数
    code += generateFunCode(funQStr);

    code += R"(// 定义一个存储函数指针的数组
string (*funcArray[])() = { )";
    
    for (auto number : funNumber) {
        code += "fun";
        code += QString::number(number);
        code += ",";
    }

    code.chop(1);
    code += R"( };

void process(const KeyValue& line) {
    string key = line.key;
    string value = line.value;

    // 拿到当前栈的状态
    int state = stateStack.top();

    

    // 找到下一个状态字符串
    string nextStateStr = SLRVector[state].m[key == "EOF" ? "$" : key];
    int nextState;
    switch (nextStateStr[0]) {
    case 's':   // 下一步
        // 放入字符栈
        strStack.push(line);
        nextState = stoi(nextStateStr.substr(1));
        stateStack.push(nextState);
        break;
    case 'r':   // 要规约了
    {
        string res;
        size_t startPos = nextStateStr.find("("); // 查找左括号的位置)";
    code += "\n";
    code += "\t\tsize_t endPos = nextStateStr.find(\")\");   // 查找右括号的位置\n";

    code += R"(
        // 如果找到了左右括号
        if (startPos != string::npos && endPos != string::npos) {
            startPos++; // 从左括号的下一个位置开始提取内容
            res = nextStateStr.substr(startPos, endPos - startPos);
        }
        int i = grammarMap[res];
        string left = funcArray[i]();   // 调用对应的函数
        state = stateStack.top();
        nextStateStr = SLRVector[state].m[left];
        nextState = stoi(nextStateStr);
        stateStack.push(nextState);
        strStack.push(KeyValue(left));
        // 继续放入当前字符
        process(line);
        break;
    }
    case 'A':   // ACCEPT
        // 生成语法树
        cout << "成功！";
        break;
    default:
        cout << "状态表出错！";
        
    }

}

int main() {

    ifstream file(")";
    code += filePath;
    code += R"(/SLR1Str.txt");
    stringstream buffer;
    buffer << file.rdbuf();
    string SLR1Str = buffer.str();
)";

    code += R"(
    SLRVector = StringToSLRVector(SLR1Str);

    // 初始状态为0
    int state = 0;
    stateStack.push(state);
    vector<KeyValue> keyValuePairs = readKeyValuePairs(")";

    code += lexFilePath;
    code += R"(");
    for (const auto& pair : keyValuePairs) {
        process(pair);
    }
    
    string str = BTreeNodeToString(treeStack.top());
    // 将自定义格式的字符串解析为 BTreeNode
    istringstream iss(str);
    ofstream outFile(")";

    code += filePath;

    code += R"(/tree.out");
    if (outFile.is_open())
    {
        outFile << str;
        outFile.close();
    }

    return 0;
})";

    return code;
}

// 外部定义语法树节点结构（供widget.cpp使用）
struct BTreeNode {
    string kind;
    string value;
    vector<BTreeNode*> nodeList;
    BTreeNode(string kind, string val) : kind(kind), value(val) {}
};

/*
* @brief 将自定义格式的字符串解析成 BTreeNode
*/
BTreeNode* StringToBTreeNode(istringstream& iss, int depth = 0)
{
    string line;
    if (depth == 0) getline(iss, line); // 读取节点开始标记
    getline(iss, line); // 读取{
    getline(iss, line); // 读取 kind
    string kind = line.substr(10 + 4 * depth); // 跳过 "    kind: "
    getline(iss, line); // 读取 value
    string value = line.substr(11 + 4 * depth); // 跳过 "    value: "

    BTreeNode* node = new BTreeNode(kind, value);

    while (getline(iss, line))
    {
        if (line.find("nodeList") != string::npos)
        {
            // 读取子节点列表
            while (getline(iss, line))
            {
                if (line.find("BTreeNode") != string::npos) // 子节点开始标记
                {

                    node->nodeList.push_back(StringToBTreeNode(iss, depth + 1)); // 递归解析子节点
                }
                else if (line.find("}") != string::npos)
                {
                    break; // 当前节点结束
                }
            }
            break;
        }
        else if (line.find("}") != string::npos)
        {
            break; // 当前节点结束
        }
    }

    return node;
}

/*
* @brief 递归函数，用于将树形结构添加到QTreeWidget中e
*/
void populateTreeWidget(QTreeWidgetItem* parentItem, BTreeNode* node) {
    // 创建一个新的树形节点
    QTreeWidgetItem* item = new QTreeWidgetItem(parentItem);
    // 设置节点内容
    if (node->kind == "-1") {
        item->setText(0, QString::fromStdString(node->value));
    }
    else if (node->kind == "-2") {
        if (node->nodeList.size() == 0) {
            item->setText(0, QString::fromStdString("ϵ :" + node->value));
        }
        else {
            item->setText(0, QString::fromStdString(node->value));
        }
    }
    else item->setText(0, QString::fromStdString(node->kind + ":" + node->value));

    // 递归地处理子节点
    for (BTreeNode* childNode : node->nodeList) {
        populateTreeWidget(item, childNode);
    }
}


/******************** LR(1) 分析器 ***************************/

// LR(1) 项目结构 [A -> α·β, a]
struct LR1Item
{
    int gid;           // 文法编号
    int dotPos;        // 点的位置
    set<string> lookahead;  // 向前看符号集合

    bool operator<(const LR1Item& other) const
    {
        if (gid != other.gid) return gid < other.gid;
        if (dotPos != other.dotPos) return dotPos < other.dotPos;
        return lookahead < other.lookahead;
    }

    bool operator==(const LR1Item& other) const
    {
        return gid == other.gid && dotPos == other.dotPos && lookahead == other.lookahead;
    }
};

// LR(1) 状态
struct LR1State
{
    int sid;
    set<LR1Item> items;
    map<string, int> transitions;  // 转移表
};

// LR(1) 状态集合
vector<LR1State> lr1States;
int lr1StateCnt = 0;

// LR(1) 分析表
struct LR1TableUnit
{
    map<string, string> action;  // ACTION表
    map<string, int> gotoTable;  // GOTO表
};
vector<LR1TableUnit> LR1Table;

// LR(1) 结果提示
QString LR1Result;

/*
* @brief 计算符号串的FIRST集合
* @param symbols 符号串（用空格分隔）
* @param startPos 开始位置
* @return FIRST集合
*/
set<string> getFirstOfString(const QStringList& symbols, int startPos)
{
    set<string> result;

    for (int i = startPos; i < symbols.size(); ++i)
    {
        string sym = symbols[i].toStdString();

        if (sym == "@")
        {
            continue;  // 空串，继续看下一个
        }
        else if (isSmallAlpha(sym))
        {
            result.insert(sym);
            return result;  // 终结符，直接返回
        }
        else if (isBigAlpha(sym))
        {
            // 非终结符，加入其FIRST集合（不包含ε）
            for (const string& s : firstSets[sym].s)
            {
                result.insert(s);
            }
            // 如果不包含空串，停止
            if (!firstSets[sym].isEpsilon)
            {
                return result;
            }
        }
    }

    // 如果所有符号都可以推导出ε，加入ε
    result.insert("@");
    return result;
}

/*
* @brief 计算向前看符号
* @param item 当前项目
* @param lookahead 当前向前看符号
* @return 新的向前看符号集合
*/
set<string> computeLookahead(const LR1Item& item, const string& la)
{
    set<string> result;

    // 获取产生式右部
    QStringList rightList = QString::fromStdString(grammarDeque[item.gid].right).split(" ");

    // 计算 FIRST(βa)，其中β是点后面的符号串，a是原向前看符号
    QStringList beta;
    for (int i = item.dotPos + 1; i < rightList.size(); ++i)
    {
        beta.append(rightList[i]);
    }
    beta.append(QString::fromStdString(la));

    set<string> firstBeta = getFirstOfString(beta, 0);

    for (const string& s : firstBeta)
    {
        if (s != "@")
        {
            result.insert(s);
        }
    }

    // 如果β可以推导出空串，加入原向前看符号
    bool allNullable = true;
    for (int i = item.dotPos + 1; i < rightList.size(); ++i)
    {
        string sym = rightList[i].toStdString();
        if (isSmallAlpha(sym))
        {
            allNullable = false;
            break;
        }
        else if (isBigAlpha(sym) && !firstSets[sym].isEpsilon)
        {
            allNullable = false;
            break;
        }
    }

    if (allNullable || item.dotPos + 1 >= rightList.size())
    {
        result.insert(la);
    }

    return result;
}

/*
* @brief LR(1) 项目集闭包
*/
set<LR1Item> lr1Closure(const set<LR1Item>& items)
{
    set<LR1Item> closure = items;
    bool changed = true;

    while (changed)
    {
        changed = false;
        set<LR1Item> newItems;

        for (const LR1Item& item : closure)
        {
            QStringList rightList = QString::fromStdString(grammarDeque[item.gid].right).split(" ");

            // 如果点在末尾或者是空产生式，跳过
            if (item.dotPos >= rightList.size() || grammarDeque[item.gid].right == "@")
            {
                continue;
            }

            string nextSymbol = rightList[item.dotPos].toStdString();

            // 如果点后面是非终结符
            if (isBigAlpha(nextSymbol))
            {
                // 对于该非终结符的每个产生式
                for (const string& production : grammarMap[nextSymbol])
                {
                    int prodGid = grammarToInt[make_pair(nextSymbol, production)];

                    // 计算新的向前看符号
                    for (const string& la : item.lookahead)
                    {
                        set<string> newLookahead = computeLookahead(item, la);

                        // 创建新项目
                        LR1Item newItem;
                        newItem.gid = prodGid;
                        newItem.dotPos = 0;
                        newItem.lookahead = newLookahead;

                        // 检查是否已存在相同的项目（合并向前看符号）
                        bool found = false;
                        for (auto& existItem : newItems)
                        {
                            if (existItem.gid == newItem.gid && existItem.dotPos == newItem.dotPos)
                            {
                                // 合并向前看符号
                                LR1Item mergedItem = existItem;
                                for (const string& s : newItem.lookahead)
                                {
                                    mergedItem.lookahead.insert(s);
                                }
                                if (mergedItem.lookahead.size() > existItem.lookahead.size())
                                {
                                    newItems.erase(existItem);
                                    newItems.insert(mergedItem);
                                    changed = true;
                                }
                                found = true;
                                break;
                            }
                        }

                        if (!found)
                        {
                            // 检查closure中是否存在
                            bool foundInClosure = false;
                            for (auto& existItem : closure)
                            {
                                if (existItem.gid == newItem.gid && existItem.dotPos == newItem.dotPos)
                                {
                                    // 检查向前看符号是否已包含
                                    bool allContained = true;
                                    for (const string& s : newItem.lookahead)
                                    {
                                        if (existItem.lookahead.find(s) == existItem.lookahead.end())
                                        {
                                            allContained = false;
                                            break;
                                        }
                                    }
                                    if (!allContained)
                                    {
                                        newItems.insert(newItem);
                                        changed = true;
                                    }
                                    foundInClosure = true;
                                    break;
                                }
                            }
                            if (!foundInClosure)
                            {
                                newItems.insert(newItem);
                                changed = true;
                            }
                        }
                    }
                }
            }
        }

        // 合并新项目到closure中
        for (const LR1Item& newItem : newItems)
        {
            bool merged = false;
            set<LR1Item> updatedClosure;
            for (auto& existItem : closure)
            {
                if (existItem.gid == newItem.gid && existItem.dotPos == newItem.dotPos)
                {
                    LR1Item mergedItem = existItem;
                    for (const string& s : newItem.lookahead)
                    {
                        mergedItem.lookahead.insert(s);
                    }
                    updatedClosure.insert(mergedItem);
                    merged = true;
                }
                else
                {
                    updatedClosure.insert(existItem);
                }
            }
            if (!merged)
            {
                updatedClosure.insert(newItem);
            }
            closure = updatedClosure;
        }
    }

    return closure;
}

/*
* @brief 计算GOTO(I, X)
*/
set<LR1Item> lr1Goto(const set<LR1Item>& items, const string& symbol)
{
    set<LR1Item> result;

    for (const LR1Item& item : items)
    {
        QStringList rightList = QString::fromStdString(grammarDeque[item.gid].right).split(" ");

        if (item.dotPos < rightList.size() && grammarDeque[item.gid].right != "@")
        {
            string nextSymbol = rightList[item.dotPos].toStdString();

            if (nextSymbol == symbol)
            {
                LR1Item newItem;
                newItem.gid = item.gid;
                newItem.dotPos = item.dotPos + 1;
                newItem.lookahead = item.lookahead;

                // 合并向前看符号
                bool found = false;
                set<LR1Item> updatedResult;
                for (auto& existItem : result)
                {
                    if (existItem.gid == newItem.gid && existItem.dotPos == newItem.dotPos)
                    {
                        LR1Item mergedItem = existItem;
                        for (const string& s : newItem.lookahead)
                        {
                            mergedItem.lookahead.insert(s);
                        }
                        updatedResult.insert(mergedItem);
                        found = true;
                    }
                    else
                    {
                        updatedResult.insert(existItem);
                    }
                }
                if (!found)
                {
                    updatedResult.insert(newItem);
                }
                result = updatedResult;
            }
        }
    }

    return lr1Closure(result);
}

/*
* @brief 比较两个LR1状态的核心项目是否相同（用于查找状态）
*/
bool compareLR1States(const set<LR1Item>& s1, const set<LR1Item>& s2)
{
    if (s1.size() != s2.size()) return false;

    for (const auto& item1 : s1)
    {
        bool found = false;
        for (const auto& item2 : s2)
        {
            if (item1.gid == item2.gid && item1.dotPos == item2.dotPos &&
                item1.lookahead == item2.lookahead)
            {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }
    return true;
}

/*
* @brief 查找LR1状态
* @return 状态id，如果不存在返回-1
*/
int findLR1State(const set<LR1Item>& items)
{
    for (const LR1State& state : lr1States)
    {
        if (compareLR1States(state.items, items))
        {
            return state.sid;
        }
    }
    return -1;
}

/*
* @brief 生成LR(1)自动机
*/
void generateLR1Automaton()
{
    lr1States.clear();
    lr1StateCnt = 0;
    LR1Result.clear();

    // 创建初始状态
    LR1Item startItem;
    startItem.gid = 0;  // 增广文法的第一条
    startItem.dotPos = 0;
    startItem.lookahead.insert("$");

    set<LR1Item> startItems;
    startItems.insert(startItem);
    set<LR1Item> startClosure = lr1Closure(startItems);

    LR1State startState;
    startState.sid = lr1StateCnt++;
    startState.items = startClosure;
    lr1States.push_back(startState);

    // 使用队列进行BFS
    queue<int> stateQueue;
    stateQueue.push(0);

    // 收集所有符号
    set<string> allSymbols;
    for (const string& s : smallAlpha) allSymbols.insert(s);
    for (const string& s : bigAlpha) allSymbols.insert(s);

    while (!stateQueue.empty())
    {
        int currentSid = stateQueue.front();
        stateQueue.pop();

        LR1State& currentState = lr1States[currentSid];

        for (const string& symbol : allSymbols)
        {
            set<LR1Item> gotoItems = lr1Goto(currentState.items, symbol);

            if (gotoItems.empty()) continue;

            int existingState = findLR1State(gotoItems);

            if (existingState == -1)
            {
                // 创建新状态
                LR1State newState;
                newState.sid = lr1StateCnt++;
                newState.items = gotoItems;
                lr1States.push_back(newState);

                lr1States[currentSid].transitions[symbol] = newState.sid;
                stateQueue.push(newState.sid);
            }
            else
            {
                lr1States[currentSid].transitions[symbol] = existingState;
            }
        }
    }

    LR1Result += QString::fromStdString("LR(1)自动机构建完成，共 " + to_string(lr1States.size()) + " 个状态\n");
}

/*
* @brief 获取LR1状态的文法字符串表示
*/
string getLR1StateGrammar(const LR1State& state)
{
    string result = "";
    for (const LR1Item& item : state.items)
    {
        const grammarUnit& g = grammarDeque[item.gid];
        string r = g.left == "zengguang" ? "E'" : g.left;
        r += "->";

        QStringList rightList = QString::fromStdString(g.right).split(" ");
        string rightResult = "";

        if (g.right == "@")
        {
            rightResult = ".";
        }
        else
        {
            for (int i = 0; i <= rightList.size(); ++i)
            {
                if (i == item.dotPos) rightResult += ".";
                if (i < rightList.size()) rightResult += rightList[i].toStdString();
            }
        }

        r += rightResult + ", {";

        bool first = true;
        for (const string& la : item.lookahead)
        {
            if (!first) r += "/";
            r += la;
            first = false;
        }
        r += "} ";

        result += r + "\n";
    }
    return result;
}

/*
* @brief 生成LR(1)分析表
* @return 0: 成功, 1: 移进-规约冲突, 2: 规约-规约冲突, 3: 两者都有
*/
int generateLR1Table()
{
    LR1Table.clear();
    LR1Table.resize(lr1States.size());

    int conflictType = 0;

    for (const LR1State& state : lr1States)
    {
        LR1TableUnit& tableUnit = LR1Table[state.sid];

        // 处理移进和GOTO
        for (const auto& trans : state.transitions)
        {
            const string& symbol = trans.first;
            int nextState = trans.second;

            if (isSmallAlpha(symbol))
            {
                // ACTION表：移进
                if (tableUnit.action.find(symbol) != tableUnit.action.end())
                {
                    // 移进-规约冲突
                    if (tableUnit.action[symbol][0] == 'r')
                    {
                        conflictType |= 1;
                        LR1Result += QString::fromStdString("状态" + to_string(state.sid) +
                                                           "在符号'" + symbol + "'上存在移进-规约冲突\n");
                    }
                }
                tableUnit.action[symbol] = "s" + to_string(nextState);
            }
            else if (isBigAlpha(symbol))
            {
                // GOTO表
                tableUnit.gotoTable[symbol] = nextState;
            }
        }

        // 处理规约
        for (const LR1Item& item : state.items)
        {
            QStringList rightList = QString::fromStdString(grammarDeque[item.gid].right).split(" ");

            // 检查是否是规约项目（点在末尾）
            bool isReduceItem = (item.dotPos >= rightList.size()) ||
                                (grammarDeque[item.gid].right == "@");

            if (isReduceItem)
            {
                for (const string& la : item.lookahead)
                {
                    if (grammarDeque[item.gid].left == trueStartSymbol && la == "$")
                    {
                        // 接受
                        if (tableUnit.action.find(la) != tableUnit.action.end() &&
                            tableUnit.action[la] != "ACCEPT")
                        {
                            conflictType |= 1;
                        }
                        tableUnit.action[la] = "ACCEPT";
                    }
                    else
                    {
                        string reduceAction = "r(" + grammarDeque[item.gid].left + "->" +
                                             grammarDeque[item.gid].right + ")";

                        if (tableUnit.action.find(la) != tableUnit.action.end())
                        {
                            if (tableUnit.action[la][0] == 's')
                            {
                                // 移进-规约冲突
                                conflictType |= 1;
                                LR1Result += QString::fromStdString("状态" + to_string(state.sid) +
                                                                   "在符号'" + la + "'上存在移进-规约冲突\n");
                            }
                            else if (tableUnit.action[la][0] == 'r' && tableUnit.action[la] != reduceAction)
                            {
                                // 规约-规约冲突
                                conflictType |= 2;
                                LR1Result += QString::fromStdString("状态" + to_string(state.sid) +
                                                                   "在符号'" + la + "'上存在规约-规约冲突\n");
                            }
                        }
                        else
                        {
                            tableUnit.action[la] = reduceAction;
                        }
                    }
                }
            }
        }
    }

    return conflictType;
}

// LR(1)相关的终结符和非终结符集合
set<string> LR1_VT;
set<string> LR1_VN;

/*
* @brief 收集LR(1)用到的符号
*/
void collectLR1Symbols()
{
    LR1_VT.clear();
    LR1_VN.clear();

    for (const LR1State& state : lr1States)
    {
        for (const auto& trans : state.transitions)
        {
            if (isSmallAlpha(trans.first))
            {
                LR1_VT.insert(trans.first);
            }
            else if (isBigAlpha(trans.first))
            {
                LR1_VN.insert(trans.first);
            }
        }
    }
}

/*
* @brief 清空全局变量
*/
void reset()
{
    grammarMap.clear();

    bigAlpha.clear();
    smallAlpha.clear();


    firstSets.clear();
    followSets.clear();
    LR0Result.clear();
    grammarDeque.clear();
    dfaStateVector.clear();
    dfaCellVector.clear();
    VT.clear();
    VN.clear();
    SLRVector.clear();
    scnt = 0;
    ccnt = 0;
    funCodeError = 0;
    funNumber.clear();

    // 清空LR(1)相关变量
    lr1States.clear();
    lr1StateCnt = 0;
    LR1Table.clear();
    LR1Result.clear();
    LR1_VT.clear();
    LR1_VN.clear();
}

/******************** UI界面 ***************************/
// 查看输入规则
void Widget::on_pushButton_7_clicked()
{
    QString message = R"(* 第一行输入非终结符，用|隔开
* 第二行输入终结符，用|隔开
* 输入时不同的单词、标识符请用空格隔开；
* 用@表示空串，默认左边出现的第一个单词、标识符为文法的开始符号
* 同时，文法中含有或(|)，请分开两条输入)";

    QMessageBox::information(this, "输入规则", message);
}

// 打开文法规则
void Widget::on_pushButton_3_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("选择文件"), QDir::homePath(), tr("文本文件 (*.txt);;所有文件 (*.*)"));

    if (!filePath.isEmpty())
    {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::critical(this, "错误信息", "导入错误！无法打开文件，请检查路径和文件是否被占用！");
            return;
        }
        QTextStream in(&file);
        in.setCodec("UTF-8");
        QString fileContents = in.readAll();
        file.close();
        ui->plainTextEdit_2->setPlainText(fileContents);
    }
}

// 保存文法规则
void Widget::on_pushButton_4_clicked()
{
    QString saveFilePath = QFileDialog::getSaveFileName(this, tr("保存文法文件"), QDir::homePath(), tr("文本文件 (*.txt)"));
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

// 求解first集合按钮
void Widget::on_pushButton_5_clicked()
{
    reset();
    QString grammar_q = ui->plainTextEdit_2->toPlainText();
    grammarStr = grammar_q.toStdString();
    if (grammar_q.isEmpty()) {
        QMessageBox::critical(this, "错误信息", "请先输入文法");
        return;
    }
    handleGrammar();
    getFirstSets();

    QTableWidget* tableWidget = ui->tableWidget_3;

    // 清空表格内容
    tableWidget->clearContents();

    // 设置表格的列数
    tableWidget->setColumnCount(2);

    // 设置表头
    QStringList headerLabels;
    headerLabels << "非终结符" << "First集合";
    tableWidget->setHorizontalHeaderLabels(headerLabels);

    // 设置行数
    tableWidget->setRowCount(firstSets.size());

    // 遍历非终结符的First集合，将其展示在表格中
    int row = 0;
    for (const auto& entry : firstSets)
    {
        string nonTerminal = entry.first;
        const set<string>& firstSet = entry.second.s;

        // 在表格中设置非终结符
        QTableWidgetItem* nonTerminalItem = new QTableWidgetItem(QString::fromStdString(nonTerminal));
        tableWidget->setItem(row, 0, nonTerminalItem);

        // 在表格中设置First集合，将set<char>转换为逗号分隔的字符串
        QString firstSetString;
        for (string symbol : firstSet)
        {
            firstSetString += QString::fromStdString(symbol) + ",";
        }
        if (entry.second.isEpsilon)
        {
            firstSetString += QString('@') + ",";
        }
        // 去掉最后一个逗号
        if (!firstSetString.isEmpty())
        {
            firstSetString.chop(1);
        }

        QTableWidgetItem* firstSetItem = new QTableWidgetItem(firstSetString);
        tableWidget->setItem(row, 1, firstSetItem);

        // 增加行数
        ++row;
    }
}

// 求解follow集合按钮
void Widget::on_pushButton_6_clicked()
{
    reset();
    QString grammar_q = ui->plainTextEdit_2->toPlainText();
    grammarStr = grammar_q.toStdString();
    if (grammar_q.isEmpty()) {
        QMessageBox::critical(this, "错误信息", "请先输入文法");
        return;
    }
    handleGrammar();
    getFirstSets();
    getFollowSets();


    // 清空TableWidget
    ui->tableWidget_4->clear();

    // 设置表格的行数和列数
    int rowCount = followSets.size();
    int columnCount = 2; // 两列
    ui->tableWidget_4->setRowCount(rowCount);
    ui->tableWidget_4->setColumnCount(columnCount);

    // 设置表头
    QStringList headers;
    headers << "非终结符" << "Follow集合";
    ui->tableWidget_4->setHorizontalHeaderLabels(headers);

    // 遍历followSets，将数据填充到TableWidget中
    int row = 0;
    for (const auto& entry : followSets) {
        // 获取非终结符和对应的followUnit
        string nonTerminal = entry.first;
        const followUnit& followSet = entry.second;

        // 在第一列设置非终结符
        QTableWidgetItem* nonTerminalItem = new QTableWidgetItem(QString::fromStdString(nonTerminal));
        ui->tableWidget_4->setItem(row, 0, nonTerminalItem);

        // 在第二列设置followUnit，使用逗号拼接
        QString followSetStr = "";
        for (string c : followSet.s) {
            followSetStr += QString::fromStdString(c);
            followSetStr += ",";
        }
        followSetStr.chop(1); // 移除最后一个逗号
        QTableWidgetItem* followSetItem = new QTableWidgetItem(followSetStr);
        ui->tableWidget_4->setItem(row, 1, followSetItem);

        // 移动到下一行
        ++row;
    }
}

// 生成LR(0)DFA图
void Widget::on_pushButton_clicked()
{
    reset();
    ui->tableWidget->clear();
    QString grammar_q = ui->plainTextEdit_2->toPlainText();
    grammarStr = grammar_q.toStdString();
    if (grammar_q.isEmpty()) {
        QMessageBox::critical(this, "错误信息", "请先输入文法");
        return;
    }
    handleGrammar();
    // LR0Result 已不再显示在界面上
    getLR0();

    int numRows = dfaStateVector.size();
    int numCols = 2 + VT.size() + VN.size();

    ui->tableWidget->setRowCount(numRows);
    ui->tableWidget->setColumnCount(numCols);

    // Set the table headers
    QStringList headers;
    headers << "状态" << "状态内文法";
    map<string, int> c2int;
    int cnt = 0;
    for (string vt : VT) {
        headers << QString::fromStdString(vt);
        c2int[vt] = cnt++;
    }
    for (string vn : VN) {
        headers << QString::fromStdString(vn);
        c2int[vn] = cnt++;
    }
    ui->tableWidget->setHorizontalHeaderLabels(headers);

    // Populate the table with data
    for (int i = 0; i < numRows; ++i)
    {
        ui->tableWidget->setItem(i, 0, new QTableWidgetItem(QString::number(dfaStateVector[i].sid)));
        ui->tableWidget->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(getStateGrammar(dfaStateVector[i]))));

        // Display nextStateVector
        for (int j = 0; j < dfaStateVector[i].nextStateVector.size(); ++j)
        {
            ui->tableWidget->setItem(i, 2 + c2int[dfaStateVector[i].nextStateVector[j].c], new QTableWidgetItem(QString::number(dfaStateVector[i].nextStateVector[j].sid)));
        }
    }
}

// 分析SLR(1)文法
void Widget::on_pushButton_2_clicked()
{
    reset();
    QString grammar_q = ui->plainTextEdit_2->toPlainText();
    grammarStr = grammar_q.toStdString();
    if (grammar_q.isEmpty()) {
        QMessageBox::critical(this, "错误信息", "请先输入文法");
        return;
    }
    handleGrammar();
    getFirstSets();
    getFollowSets();
    getLR0();
    int result = getSLR1Table();
    QString resultMsg;
    switch (result)
    {

    case 2:
        QMessageBox::warning(this, "分析结果", "出现归约-归约冲突");
        break;
    case 3:
        QMessageBox::warning(this, "分析结果", "出现归约-移进冲突和归约-归约冲突");
        break;
    case 1:
        resultMsg = "出现归约-移进冲突，只做移进不做规约，得到SLR1分析表";
    case 0:
    {
        if (result == 0)
            resultMsg = "符合SLR(1)文法，请查看SLR(1)分析表！";
        QMessageBox::information(this, "分析结果", resultMsg);
        ui->tableWidget_2->clear();
        VT.insert("$");
        int numRows = SLRVector.size();
        int numCols = 1 + VT.size() + VN.size();

        ui->tableWidget_2->setRowCount(numRows);
        ui->tableWidget_2->setColumnCount(numCols);
        // Set the table headers
        QStringList headers;
        headers << "状态";
        map<string, int> c2int;
        int cnt = 0;
        for (string vt : VT) {
            headers << QString::fromStdString(vt);
            c2int[vt] = cnt++;
        }
        for (string vn : VN) {
            headers << QString::fromStdString(vn);
            c2int[vn] = cnt++;
        }
        ui->tableWidget_2->setHorizontalHeaderLabels(headers);

        // Populate the table with data
        for (int i = 0; i < numRows; ++i)
        {
            ui->tableWidget_2->setItem(i, 0, new QTableWidgetItem(QString::number(i)));

            // Display nextStateVector
            for (const auto& slrunit : SLRVector[i].m)
            {
                ui->tableWidget_2->setItem(i, 1 + c2int[slrunit.first], new QTableWidgetItem(QString::fromStdString(slrunit.second)));
            }
        }

        for (int i = 0; i < numRows; ++i)
        {
            cout << "Row " << i << ":";

            // Print the state
            cout << i;

            // Display nextStateVector
            for (int j = 0; j < numCols; ++j)
            {
                QTableWidgetItem* item = ui->tableWidget_2->item(i, j);
                if (item != nullptr)
                {
                    cout << item->text().toStdString();
                }
                else
                {
                    cout << "\t";
                }
            }
        }

        break;
    }

    }
}

// 生成代码按钮
void Widget::on_pushButton_8_clicked()
{
    QString srcFilePath;
    QString t_filePath = QFileDialog::getExistingDirectory(this, "选择lex文件所在的文件夹路径", QDir::currentPath());
    if (t_filePath.isEmpty())
        return;
    else
        srcFilePath = t_filePath;

    // 选择lex文件
    QString lexFilePath = QFileDialog::getOpenFileName(this, tr("选择词法分析输出文件"), srcFilePath, tr("Lex文件 (*.lex);;所有文件 (*.*)"));
    if (lexFilePath.isEmpty()) {
        QMessageBox::warning(this, "提示", "未选择词法分析文件");
        return;
    }

    QString filePath = QFileDialog::getOpenFileName(this, tr("选择语义函数文件"), QDir::homePath(), tr("文本文件 (*.txt);;所有文件 (*.*)"));
    if (!filePath.isEmpty())
    {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::critical(this, "错误信息", "导入错误！无法打开文件，请检查路径和文件是否被占用！");
            return;
        }
        QTextStream in(&file);
        in.setCodec("UTF-8");
        QString funQString = in.readAll(); // 语义函数QString
        file.close();

        qDebug() << funQString;

        reset();
        QString grammar_q = ui->plainTextEdit_2->toPlainText();
        if (grammar_q.isEmpty()) {
            QMessageBox::critical(this, "错误信息", "请先输入文法");
            return;
        }
        grammarStr = grammar_q.toStdString();
        handleGrammar();
        getFirstSets();
        getFollowSets();
        getLR0();
        int result = getSLR1Table();


        QString treeCode = generateTreeCode(srcFilePath, lexFilePath, funQString);
        if (funCodeError == 1) {
            QMessageBox::critical(this, "错误信息", "语义函数行数和文法行数不一致，请检查！");
        }

        ui->codeText->setPlainText(treeCode);

        // 将 SLRVector 转换为自定义格式的字符串
        string SLR1Str = SLRVectorToString(SLRVector);
        QFile tgtFile1(srcFilePath + "/SLR1Str.txt");
        if (!tgtFile1.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate))
        {
            QMessageBox::warning(NULL, "文件", "文件打开/写入失败");
            return;
        }
        QTextStream outputFile1(&tgtFile1);
        outputFile1 << QString::fromStdString(SLR1Str);
        tgtFile1.close();

        QFile tgtFile(srcFilePath + "/treeCode.cpp");
        if (!tgtFile.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate))
        {
            QMessageBox::warning(NULL, "文件", "文件打开/写入失败");
            return;
        }
        QTextStream outputFile(&tgtFile);
        //QString tgStr=ui->plainTextEdit_Lexer->toPlainText();
        outputFile << treeCode;
        tgtFile.close();
    }


}

// 语法树可视化按钮
void Widget::on_pushButton_9_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("选择输出树形文件"), QDir::homePath(), tr("文本文件 (*.out);;所有文件 (*.*)"));

    if (!filePath.isEmpty())
    {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::critical(this, "错误信息", "导入错误！无法打开文件，请检查路径和文件是否被占用！");
            return;
        }
        QTextStream in(&file);
        in.setCodec("UTF-8");
        QString fileContents = in.readAll();
        file.close();
        istringstream iss(fileContents.toStdString());
        BTreeNode* rootNode = StringToBTreeNode(iss);

        ui->treeWidget->clear(); // 清空所有顶级项目

        // 创建根节点
        QTreeWidgetItem* rootItem = new QTreeWidgetItem();
        rootItem->setText(0, "Root");

        populateTreeWidget(rootItem, rootNode);

        ui->treeWidget->addTopLevelItem(rootItem);
        // 默认展开第一层
        if (ui->treeWidget->topLevelItemCount() > 0) {
            QTreeWidgetItem* firstTopLevelItem = ui->treeWidget->topLevelItem(0);
            firstTopLevelItem->setExpanded(true);
        }
    }
}

// 生成LR(1) DFA图
void Widget::on_pushButton_10_clicked()
{
    reset();
    ui->tableWidget_5->clear();
    QString grammar_q = ui->plainTextEdit_2->toPlainText();
    grammarStr = grammar_q.toStdString();
    if (grammar_q.isEmpty()) {
        QMessageBox::critical(this, "错误信息", "请先输入文法");
        return;
    }

    handleGrammar();
    getFirstSets();
    getFollowSets();

    // 生成LR(1)自动机
    generateLR1Automaton();

    // 弹窗显示LR(1)自动机构建结果
    QMessageBox::information(this, "LR(1) DFA生成结果", LR1Result);

    // 收集符号
    collectLR1Symbols();

    // 显示LR(1) DFA表格
    int numRows = lr1States.size();
    int numCols = 2 + LR1_VT.size() + LR1_VN.size();

    ui->tableWidget_5->setRowCount(numRows);
    ui->tableWidget_5->setColumnCount(numCols);

    // 设置表头
    QStringList headers;
    headers << "状态" << "状态内项目";
    map<string, int> c2int;
    int cnt = 0;
    for (const string& vt : LR1_VT) {
        headers << QString::fromStdString(vt);
        c2int[vt] = cnt++;
    }
    for (const string& vn : LR1_VN) {
        headers << QString::fromStdString(vn);
        c2int[vn] = cnt++;
    }
    ui->tableWidget_5->setHorizontalHeaderLabels(headers);

    // 填充表格数据
    for (int i = 0; i < numRows; ++i)
    {
        ui->tableWidget_5->setItem(i, 0, new QTableWidgetItem(QString::number(lr1States[i].sid)));
        ui->tableWidget_5->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(getLR1StateGrammar(lr1States[i]))));

        // 显示转移
        for (const auto& trans : lr1States[i].transitions)
        {
            if (c2int.find(trans.first) != c2int.end())
            {
                ui->tableWidget_5->setItem(i, 2 + c2int[trans.first],
                    new QTableWidgetItem(QString::number(trans.second)));
            }
        }
    }

    // 自动调整列宽
    ui->tableWidget_5->resizeColumnsToContents();
}

// 生成LR(1)分析表
void Widget::on_pushButton_11_clicked()
{
    reset();
    QString grammar_q = ui->plainTextEdit_2->toPlainText();
    grammarStr = grammar_q.toStdString();
    if (grammar_q.isEmpty()) {
        QMessageBox::critical(this, "错误信息", "请先输入文法");
        return;
    }

    handleGrammar();
    getFirstSets();
    getFollowSets();

    // 生成LR(1)自动机
    generateLR1Automaton();

    // 收集符号
    collectLR1Symbols();

    // 生成LR(1)分析表
    int result = generateLR1Table();

    QString resultMsg;
    switch (result)
    {
    case 0:
        resultMsg = "成功生成LR(1)分析表！该文法是LR(1)文法。";
        break;
    case 1:
        resultMsg = "存在移进-规约冲突，该文法不是LR(1)文法。\n" + LR1Result;
        break;
    case 2:
        resultMsg = "存在规约-规约冲突，该文法不是LR(1)文法。\n" + LR1Result;
        break;
    case 3:
        resultMsg = "同时存在移进-规约冲突和规约-规约冲突，该文法不是LR(1)文法。\n" + LR1Result;
        break;
    }
    // 弹窗显示分析结果
    if (result == 0) {
        QMessageBox::information(this, "分析结果", resultMsg);
    } else {
        QMessageBox::warning(this, "分析结果", resultMsg);
    }

    // 显示LR(1)分析表
    ui->tableWidget_6->clear();
    LR1_VT.insert("$");  // 添加结束符

    int numRows = LR1Table.size();
    int numCols = 1 + LR1_VT.size() + LR1_VN.size();

    ui->tableWidget_6->setRowCount(numRows);
    ui->tableWidget_6->setColumnCount(numCols);

    // 设置表头
    QStringList headers;
    headers << "状态";
    map<string, int> c2int;
    int cnt = 0;

    // ACTION部分（终结符）
    for (const string& vt : LR1_VT) {
        headers << QString::fromStdString(vt);
        c2int[vt] = cnt++;
    }
    // GOTO部分（非终结符）
    for (const string& vn : LR1_VN) {
        headers << QString::fromStdString(vn);
        c2int[vn] = cnt++;
    }
    ui->tableWidget_6->setHorizontalHeaderLabels(headers);

    // 填充表格数据
    for (int i = 0; i < numRows; ++i)
    {
        ui->tableWidget_6->setItem(i, 0, new QTableWidgetItem(QString::number(i)));

        // ACTION表
        for (const auto& action : LR1Table[i].action)
        {
            if (c2int.find(action.first) != c2int.end())
            {
                ui->tableWidget_6->setItem(i, 1 + c2int[action.first],
                    new QTableWidgetItem(QString::fromStdString(action.second)));
            }
        }

        // GOTO表
        for (const auto& gotoEntry : LR1Table[i].gotoTable)
        {
            if (c2int.find(gotoEntry.first) != c2int.end())
            {
                ui->tableWidget_6->setItem(i, 1 + c2int[gotoEntry.first],
                    new QTableWidgetItem(QString::number(gotoEntry.second)));
            }
        }
    }

    // 自动调整列宽
    ui->tableWidget_6->resizeColumnsToContents();
}