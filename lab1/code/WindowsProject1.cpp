#include <windows.h>
#include <commdlg.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <set>
#include <map>
#include <codecvt>
#include <locale>

using namespace std;

// 转换辅助函数：宽字符路径转多字节
string wstring_to_string(const wstring& wstr) {
    int bufferSize = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    if (bufferSize <= 0) return "";

    string str(bufferSize, 0);
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, &str[0], bufferSize, NULL, NULL);
    return str;
}

// UTF-8转宽字符
wstring utf8_to_wstring(const string& str) {
    try {
        wstring_convert<codecvt_utf8<wchar_t>> converter;
        return converter.from_bytes(str);
    }
    catch (...) {
        // 处理转换错误
        return L"";
    }
}

// Rust关键字集合
const set<wstring> rustKeywords = {
    L"as", L"break", L"const", L"continue", L"crate", L"else", L"enum", L"extern",
    L"false", L"fn", L"for", L"if", L"impl", L"in", L"let", L"loop", L"match", L"mod",
    L"move", L"mut", L"pub", L"ref", L"return", L"self", L"Self", L"static", L"struct",
    L"super", L"trait", L"true", L"type", L"unsafe", L"use", L"where", L"while"
};

// 运算符集合
const set<wstring> operators = {
    L"+", L"-", L"*", L"/", L"%", L"++", L"--", L"=", L"+=", L"-=", L"*=", L"/=", L"%=",
    L"==", L"!=", L"<", L">", L"<=", L">=", L"&&", L"||", L"!", L"&", L"|", L"^", L"~",
    L"<<", L">>", L"&=", L"|=", L"^=", L"<<=", L">>=", L"?", L"=>", L".."
};

// 分隔符集合
const set<wchar_t> separators = {
    L'(', L')', L'{', L'}', L'[', L']', L';', L',', L'.', L':', L'#', L'@', L'$', L'?'
};

// 记号类型
enum TokenType1 {
    KEYWORD,
    IDENTIFIER,
    INTEGER_LITERAL,
    HEX_INTEGER_LITERAL,
    BIN_INTEGER_LITERAL,
    OCT_INTEGER_LITERAL,
    FLOAT_LITERAL,
    STRING_LITERAL,
    COMMENT,
    OPERATOR,
    SEPARATOR,
    MACRO_IDENTIFIER,
    UNKNOWN
};

// 记号结构
struct Token {
    wstring value;
    TokenType1 type;
    int line;
    int column;
};

// 转换TokenType1到字符串
wstring tokenTypeToString(TokenType1 type) {
    switch (type) {
    case KEYWORD: return L"关键字";
    case IDENTIFIER: return L"标识符";
    case INTEGER_LITERAL: return L"字面量（整数）";
    case HEX_INTEGER_LITERAL: return L"字面量（十六进制整数）";
    case BIN_INTEGER_LITERAL: return L"字面量（二进制整数）";
    case OCT_INTEGER_LITERAL: return L"字面量（八进制整数）";
    case FLOAT_LITERAL: return L"字面量（浮点数）";
    case STRING_LITERAL: return L"字符串字面量";
    case COMMENT: return L"注释";
    case OPERATOR: return L"操作符";
    case SEPARATOR: return L"分隔符";
    case MACRO_IDENTIFIER: return L"宏调用名";
    default: return L"未知类型";
    }
}

// 读取文件内容（增强错误处理）
wstring readFile(const wstring& filename) {
    // 转换宽字符路径为多字节
    string filePath = wstring_to_string(filename);
    if (filePath.empty()) {
        MessageBoxW(NULL, L"文件路径转换失败", L"错误", MB_ICONERROR);
        return L"";
    }

    // 尝试打开文件
    ifstream file(filePath, ios::binary);
    if (!file.is_open()) {
        // 显示详细错误信息
        wstring errorMsg = L"无法打开文件: " + filename + L"\n错误代码: " + to_wstring(GetLastError());
        MessageBoxW(NULL, errorMsg.c_str(), L"文件错误", MB_ICONERROR);
        return L"";
    }

    // 检查文件是否为空
    file.seekg(0, ios::end);
    streampos fileSize = file.tellg();
    if (fileSize <= 0) {
        MessageBoxW(NULL, L"所选文件为空", L"警告", MB_ICONWARNING);
        return L"";
    }
    file.seekg(0, ios::beg);

    // 读取文件内容
    string content(fileSize, '\0');
    file.read(&content[0], fileSize);

    // 关闭文件
    file.close();

    // 转换为宽字符
    wstring wcontent = utf8_to_wstring(content);
    if (wcontent.empty() && fileSize > 0) {
        MessageBoxW(NULL, L"文件内容转换失败，可能不是UTF-8编码", L"编码错误", MB_ICONERROR);
    }

    return wcontent;
}

// 检查是否是标识符的起始字符
bool isIdentifierStart(wchar_t c) {
    return (c >= L'a' && c <= L'z') || (c >= L'A' && c <= L'Z') || c == L'_';
}

// 检查是否是标识符的字符
bool isIdentifierChar(wchar_t c) {
    return isIdentifierStart(c) || (c >= L'0' && c <= L'9');
}

// 检查是否是运算符的字符
bool isOperatorChar(wchar_t c) {
    wstring op(1, c);
    for (const wstring& o : operators) {
        if (o.find(c) != wstring::npos) {
            return true;
        }
    }
    return false;
}

// 解析源代码为记号列表
vector<Token> tokenize(const wstring& source) {
    vector<Token> tokens;
    int line = 1;
    int column = 1;
    size_t pos = 0;
    size_t length = source.length();

    while (pos < length) {
        wchar_t c = source[pos];

        // 跳过空白字符
        if (isspace(c)) {
            if (c == L'\n') {
                line++;
                column = 1;
            }
            else if (c != L'\r') {
                column++;
            }
            pos++;
            continue;
        }

        // 处理注释
        if (c == L'/') {
            if (pos + 1 < length && source[pos + 1] == L'/') {
                // 单行注释
                size_t start = pos;
                pos += 2;
                while (pos < length && source[pos] != L'\n') {
                    pos++;
                }
                wstring comment = source.substr(start, pos - start);
                tokens.push_back({ comment, COMMENT, line, column });
                column += static_cast<int>(pos - start);
                continue;
            }
            else if (pos + 1 < length && source[pos + 1] == L'*') {
                // 多行注释
                size_t start = pos;
                int startLine = line;
                int startColumn = column;
                pos += 2;
                while (pos + 1 < length && !(source[pos] == L'*' && source[pos + 1] == L'/')) {
                    if (source[pos] == L'\n') {
                        line++;
                        column = 1;
                    }
                    else if (source[pos] != L'\r') {
                        column++;
                    }
                    pos++;
                }
                if (pos + 1 < length) {
                    pos += 2;
                }
                wstring comment = source.substr(start, pos - start);
                tokens.push_back({ comment, COMMENT, startLine, startColumn });
                column += static_cast<int>(pos - start);
                continue;
            }
        }

        // 处理字符串字面量
        if (c == L'"') {
            size_t start = pos;
            pos++;
            while (pos < length && source[pos] != L'"') {
                if (source[pos] == L'\\' && pos + 1 < length) {
                    pos++; // 跳过转义字符
                }
                if (source[pos] == L'\n') {
                    line++;
                    column = 1;
                }
                else if (source[pos] != L'\r') {
                    column++;
                }
                pos++;
            }
            if (pos < length) {
                pos++;
            }
            wstring str = source.substr(start, pos - start);
            tokens.push_back({ str, STRING_LITERAL, line, column - static_cast<int>(pos - start) });
            continue;
        }

        // 处理标识符和关键字
        if (isIdentifierStart(c)) {
            size_t start = pos;
            pos++;
            while (pos < length && isIdentifierChar(source[pos])) {
                pos++;
            }

            // 检查是否是宏调用（以!结尾）
            if (pos < length && source[pos] == L'!') {
                wstring macroName = source.substr(start, pos - start);
                tokens.push_back({ macroName + L"!", MACRO_IDENTIFIER, line, column });
                pos++;
                column += static_cast<int>(pos - start);
                continue;
            }

            wstring identifier = source.substr(start, pos - start);
            TokenType1 type = (rustKeywords.find(identifier) != rustKeywords.end()) ? KEYWORD : IDENTIFIER;
            tokens.push_back({ identifier, type, line, column });
            column += static_cast<int>(pos - start);
            continue;
        }

        // 处理数字字面量
        if (iswdigit(c)) {
            size_t start = pos;
            pos++;

            // 检查是否是二进制、八进制或十六进制
            bool isHex = false, isBin = false, isOct = false;

            if (c == L'0' && pos < length) {
                if (source[pos] == L'x' || source[pos] == L'X') {
                    isHex = true;
                    pos++;
                    while (pos < length && (iswxdigit(source[pos]) || source[pos] == L'_')) {
                        pos++;
                    }
                }
                else if (source[pos] == L'b' || source[pos] == L'B') {
                    isBin = true;
                    pos++;
                    while (pos < length && (source[pos] == L'0' || source[pos] == L'1' || source[pos] == L'_')) {
                        pos++;
                    }
                }
                else if (source[pos] == L'o' || source[pos] == L'O') {
                    isOct = true;
                    pos++;
                    while (pos < length && (source[pos] >= L'0' && source[pos] <= L'7' || source[pos] == L'_')) {
                        pos++;
                    }
                }
            }

            // 如果不是特殊进制，检查是否是浮点数
            if (!isHex && !isBin && !isOct) {
                bool isFloat = false;

                // 处理十进制整数部分
                while (pos < length && (iswdigit(source[pos]) || source[pos] == L'_')) {
                    pos++;
                }

                // 检查是否有小数部分
                if (pos < length && source[pos] == L'.') {
                    isFloat = true;
                    pos++;
                    while (pos < length && (iswdigit(source[pos]) || source[pos] == L'_')) {
                        pos++;
                    }
                }

                // 检查是否有指数部分
                if (pos < length && (source[pos] == L'e' || source[pos] == L'E')) {
                    isFloat = true;
                    pos++;
                    if (pos < length && (source[pos] == L'+' || source[pos] == L'-')) {
                        pos++;
                    }
                    while (pos < length && (iswdigit(source[pos]) || source[pos] == L'_')) {
                        pos++;
                    }
                }

                if (isFloat) {
                    wstring num = source.substr(start, pos - start);
                    tokens.push_back({ num, FLOAT_LITERAL, line, column });
                    column += static_cast<int>(pos - start);
                    continue;
                }
            }

            // 确定整数类型
            TokenType1 type = INTEGER_LITERAL;
            if (isHex) type = HEX_INTEGER_LITERAL;
            else if (isBin) type = BIN_INTEGER_LITERAL;
            else if (isOct) type = OCT_INTEGER_LITERAL;

            wstring num = source.substr(start, pos - start);
            tokens.push_back({ num, type, line, column });
            column += static_cast<int>(pos - start);
            continue;
        }

        // 处理运算符
        if (isOperatorChar(c)) {
            // 尝试匹配最长的运算符
            wstring longestOp;
            for (const wstring& op : operators) {
                if (source.substr(pos, op.length()) == op && op.length() > longestOp.length()) {
                    longestOp = op;
                }
            }

            if (!longestOp.empty()) {
                tokens.push_back({ longestOp, OPERATOR, line, column });
                column += static_cast<int>(longestOp.length());
                pos += longestOp.length();
                continue;
            }
        }

        // 处理分隔符
        if (separators.find(c) != separators.end()) {
            tokens.push_back({ wstring(1, c), SEPARATOR, line, column });
            column++;
            pos++;
            continue;
        }

        // 未知字符
        tokens.push_back({ wstring(1, c), UNKNOWN, line, column });
        column++;
        pos++;
    }

    return tokens;
}

// 选择文件对话框
wstring openFileDialog(HWND hwnd) {
    OPENFILENAMEW ofn = { 0 };
    wchar_t szFile[260] = { 0 };

    ofn.lStructSize = sizeof(OPENFILENAMEW);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = _countof(szFile);
    ofn.lpstrFilter = L"Rust Source Files (*.rs)\0*.rs\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;
    ofn.lpstrTitle = L"选择Rust源代码文件";

    if (GetOpenFileNameW(&ofn)) {
        return ofn.lpstrFile;
    }

    // 检查是否是用户取消还是发生错误
    DWORD err = CommDlgExtendedError();
    if (err != 0) {
        wstring errorMsg = L"文件选择错误: " + to_wstring(err);
        MessageBoxW(hwnd, errorMsg.c_str(), L"错误", MB_ICONERROR);
    }

    return L"";
}

// 显示结果对话框
void showResults(HWND hwnd, const vector<Token>& tokens, const wstring& source) {
    if (tokens.empty()) {
        MessageBoxW(hwnd, L"没有解析到任何内容", L"提示", MB_ICONINFORMATION);
        return;
    }

    // 构建结果字符串：按照示例格式输出
    wstring result = L"运行结果如下：\r\n";
    
    // 按行分割源代码
    wstringstream ss(source);
    wstring line_content;
    int current_line_num = 1;
    size_t token_index = 0;

    while (getline(ss, line_content)) {
        // 移除行尾的 \r，以防万一
        if (!line_content.empty() && line_content.back() == L'\r') {
            line_content.pop_back();
        }

        // 输出原始行作为注释
        result += L"\r\n" + line_content + L"\r\n";

        // 遍历当前行的所有Token
        while (token_index < tokens.size() && tokens[token_index].line == current_line_num) {
            const Token& token = tokens[token_index];
            // 根据Token的类型和值进行格式化输出
            // 这里简化处理，直接输出缩进后的Token信息
            result += L"    " + token.value + L": " + tokenTypeToString(token.type) + L"\r\n";
            token_index++;
        }
        current_line_num++;
    }

    // 创建一个临时窗口显示结果
    HWND hResultWnd = CreateWindowExW(
        0,                      // 扩展样式
        L"#32770",              // 改用“对话框类”，支持标题栏和窗口操作
        L"Rust单词分类结果",    // 窗口标题
        WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL,  // 保留原样式（含滚动条）
        100, 100, 800, 600,     // 窗口位置（x,y）和尺寸（宽,高）
        hwnd, NULL, GetModuleHandleW(NULL), NULL  // 父窗口、菜单等参数
    );

    if (hResultWnd == NULL) {
        MessageBoxW(hwnd, L"无法创建结果窗口", L"错误", MB_ICONERROR);
        return;
    }

    // 添加编辑控件显示文本
    HWND hEdit = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        L"EDIT",
        result.c_str(),
        WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_NOHIDESEL,
        10, 10, 760, 540,
        hResultWnd,
        NULL,
        GetModuleHandleW(NULL),
        NULL
    );

    if (hEdit == NULL) {
        MessageBoxW(hResultWnd, L"无法创建文本控件", L"错误", MB_ICONERROR);
        DestroyWindow(hResultWnd);
        return;
    }

    // 设置字体
    HFONT hFont = CreateFontW(
        14, 0, 0, 0, FW_NORMAL,
        FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_SWISS,
        L"SimSun"
    );
    SendMessageW(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
}

// 窗口过程
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static vector<Token> tokens;

    switch (uMsg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_COMMAND:
        if (LOWORD(wParam) == 1) { // 打开文件按钮
            wstring filename = openFileDialog(hwnd);
            if (!filename.empty()) {
                wstring source = readFile(filename);
                if (!source.empty()) {
                    tokens = tokenize(source);
                    showResults(hwnd, tokens, source);
                }
            }
        }
        return 0;
    }

    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

// 注册窗口类
void registerWindowClass(HINSTANCE hInstance) {
    WNDCLASSW wc = { 0 };

    wc.lpfnWndProc = WindowProc;// 消息处理函数
    wc.hInstance = hInstance;
    wc.lpszClassName = L"RustTokenClassifierClass";// 窗口类名
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);// 光标

    if (!RegisterClassW(&wc)) {
        MessageBoxW(NULL, L"窗口类注册失败", L"错误", MB_ICONERROR);
    }
}

// 创建主窗口
HWND createMainWindow(HINSTANCE hInstance) {
    return CreateWindowExW(
        0,
        L"RustTokenClassifierClass",
        L"Rust单词拼装分类器",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 400,
        NULL,
        NULL,
        hInstance,
        NULL
    );
}

// 添加按钮
void addControls(HWND hwnd) {
    HWND hButton = CreateWindowW(
        L"BUTTON",
        L"打开Rust源文件",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        200, 150, 150, 30,
        hwnd,
        (HMENU)1,
        (HINSTANCE)GetWindowLongPtrW(hwnd, GWLP_HINSTANCE),
        NULL
    );

    if (hButton == NULL) {
        MessageBoxW(hwnd, L"无法创建按钮", L"错误", MB_ICONERROR);
    }
}

// 程序入口函数
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow) {
    // 注册窗口类
    registerWindowClass(hInstance);

    // 创建主窗口
    HWND hwnd = createMainWindow(hInstance);
    if (hwnd == NULL) {
        MessageBoxW(NULL, L"无法创建主窗口", L"错误", MB_ICONERROR);
        return 0;
    }

    // 添加控件
    addControls(hwnd);

    // 显示窗口
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // 消息循环
    MSG msg = { 0 };
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return static_cast<int>(msg.wParam);
}
