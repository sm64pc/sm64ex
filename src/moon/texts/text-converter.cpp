#include "text-converter.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "moon-loader.h"

extern "C" {
#include "game/segment2.h"
}

using namespace std;

map<wstring, vector<int>> unicodeTable = {
    {L"0",{0x0}},       {L"０",{0x0}},            {L"1",{0x1}},         {L"１",{0x1}},
    {L"2",{0x2}},       {L"２",{0x2}},            {L"3",{0x3}},         {L"３",{0x3}},
    {L"4",{0x4}},       {L"４",{0x4}},            {L"5",{0x5}},         {L"５",{0x5}},
    {L"6",{0x6}},       {L"６",{0x6}},            {L"7",{0x7}},         {L"７",{0x7}},
    {L"8",{0x8}},       {L"８",{0x8}},            {L"9",{0x9}},         {L"９",{0x9}},
    {L"A",{0xa}},       {L"Ａ",{0xa}},            {L"B",{0xb}},         {L"Ｂ",{0xb}},
    {L"C",{0xc}},       {L"Ｃ",{0xc}},            {L"D",{0xd}},         {L"Ｄ",{0xd}},
    {L"E",{0xe}},       {L"Ｅ",{0xe}},            {L"F",{0xf}},         {L"Ｆ",{0xf}},
    {L"G",{0x10}},      {L"Ｇ",{0x10}},           {L"H",{0x11}},        {L"Ｈ",{0x11}},
    {L"I",{0x12}},      {L"Ｉ",{0x12}},           {L"J",{0x13}},        {L"Ｊ",{0x13}},
    {L"K",{0x14}},      {L"Ｋ",{0x14}},           {L"L",{0x15}},        {L"Ｌ",{0x15}},
    {L"M",{0x16}},      {L"Ｍ",{0x16}},           {L"N",{0x17}},        {L"Ｎ",{0x17}},
    {L"O",{0x18}},      {L"Ｏ",{0x18}},           {L"P",{0x19}},        {L"Ｐ",{0x19}},
    {L"Q",{0x1a}},      {L"Ｑ",{0x1a}},           {L"R",{0x1b}},        {L"Ｒ",{0x1b}},
    {L"S",{0x1c}},      {L"Ｓ",{0x1c}},           {L"T",{0x1d}},        {L"Ｔ",{0x1d}},
    {L"U",{0x1e}},      {L"Ｕ",{0x1e}},           {L"V",{0x1f}},        {L"Ｖ",{0x1f}},
    {L"W",{0x20}},      {L"Ｗ",{0x20}},           {L"X",{0x21}},        {L"Ｘ",{0x21}},
    {L"Y",{0x22}},      {L"Ｙ",{0x22}},           {L"Z",{0x23}},        {L"Ｚ",{0x23}},
    {L"a",{0x24}},      {L"ａ",{0x24}},           {L"b",{0x25}},        {L"ｂ",{0x25}},
    {L"c",{0x26}},      {L"ｃ",{0x26}},           {L"d",{0x27}},        {L"ｄ",{0x27}},
    {L"e",{0x28}},      {L"ｅ",{0x28}},           {L"f",{0x29}},        {L"ｆ",{0x29}},
    {L"g",{0x2a}},      {L"ｇ",{0x2a}},           {L"h",{0x2b}},        {L"ｈ",{0x2b}},
    {L"i",{0x2c}},      {L"ｉ",{0x2c}},           {L"j",{0x2d}},        {L"ｊ",{0x2d}},
    {L"k",{0x2e}},      {L"ｋ",{0x2e}},           {L"l",{0x2f}},        {L"ｌ",{0x2f}},
    {L"m",{0x30}},      {L"ｍ",{0x30}},           {L"n",{0x31}},        {L"ｎ",{0x31}},
    {L"o",{0x32}},      {L"ｏ",{0x32}},           {L"p",{0x33}},        {L"ｐ",{0x33}},
    {L"q",{0x34}},      {L"ｑ",{0x34}},           {L"r",{0x35}},        {L"ｒ",{0x35}},
    {L"s",{0x36}},      {L"ｓ",{0x36}},           {L"t",{0x37}},        {L"ｔ",{0x37}},
    {L"u",{0x38}},      {L"ｕ",{0x38}},           {L"v",{0x39}},        {L"ｖ",{0x39}},
    {L"w",{0x3a}},      {L"ｗ",{0x3a}},           {L"x",{0x3b}},        {L"ｘ",{0x3b}},
    {L"y",{0x3c}},      {L"ｙ",{0x3c}},           {L"z",{0x3d}},        {L"ｚ",{0x3d}},
    {L"'",{0x3e}},      {L"＇",{0x3e}},           {L".",{0x3f}},        {L"．",{0x3f}},
    {L",",{0x6f}},      {L"，",{0x6f}},           {L"-",{0x9f}},        {L"－",{0x9f}},
    {L"(",{0xe1}},      {L"（",{0xe1}},           {L")(",{0xe2}},       {L"）（",{0xe2}},
    {L")",{0xe3}},      {L"）",{0xe3}},           {L"+",{0xe4}},        {L"＋",{0xe4}},
    {L"&",{0xe5}},      {L"＆",{0xe5}},           {L":",{0xe6}},        {L"：",{0xe6}},
    {L"!",{0xf2}},      {L"！",{0xf2}},           {L"%",{0xf3}},        {L"％",{0xf3}},
    {L"?",{0xf4}},      {L"？",{0xf4}},           {L"~",{0xf7}},        {L"～",{0xf7}},
    {L" ",{0x9e}},      {L"　",{0x9e}},         {L"\n",{0xfe}},       {L"゛",{0xf0}},
    {L"↔",{0xe4}},      {L"$",{0xf9}},          {L"★",{0xfa}},        {L"×",{0xfb}},
    {L"・",{0xfc}},     {L"☆",{0xfd}},          {L"^",{0x50}},        {L"|",{0x51}},
    {L"À",{0x44}},      {L"Á",{0x45}},          {L"Â",{0x46}},        {L"Ã",{0x47}},
    {L"Ä",{0x48}},      {L"Å",{0x49}},          {L"Æ",{0x4a}},        {L"Ç",{0x4b}},
    {L"゜",{0xf1}},       {L"『",{0xf5}},          {L"』",{0xf6}},        {L"…",{0xf8}},
    {L"È",{0x4c}},      {L"É",{0x4d}},          {L"Ê",{0x4e}},        {L"Ë",{0x4f}},
    {L"Ì",{0x59}},      {L"Í",{0x5a}},          {L"Î",{0x5b}},        {L"Ï",{0x5c}},
    {L"Ð",{0x5d}},      {L"Ñ",{0x5e}},          {L"Ò",{0x5f}},        {L"Ó",{0x60}},
    {L"Ô",{0x61}},      {L"Õ",{0x62}},          {L"Ö",{0x63}},        {L"Ø",{0x64}},
    {L"Ù",{0x65}},      {L"Ú",{0x66}},          {L"Û",{0x67}},        {L"Ü",{0x68}},
    {L"Ý",{0x69}},      {L"Þ",{0x6a}},          {L"ß",{0x6b}},        {L"à",{0x6c}},
    {L"á",{0x6d}},      {L"â",{0x6e}},          {L"ã",{0x70}},        {L"ä",{0x71}},
    {L"å",{0x72}},      {L"æ",{0x73}},          {L"ç",{0x74}},        {L"è",{0x75}},
    {L"é",{0x76}},      {L"ê",{0x77}},          {L"ë",{0x78}},        {L"ì",{0x79}},
    {L"í",{0x7a}},      {L"î",{0x7b}},          {L"ï",{0x7c}},        {L"ð",{0x7d}},
    {L"ñ",{0x7e}},      {L"ò",{0x7f}},          {L"ó",{0x80}},        {L"ô",{0x81}},
    {L"õ",{0x82}},      {L"ö",{0x83}},          {L"ø",{0x84}},        {L"ù",{0x85}},
    {L"ú",{0x86}},      {L"û",{0x87}},          {L"ü",{0x88}},        {L"ý",{0x89}},
    {L"Ğ",{0x92}},      {L"ğ",{0x93}},         {L"İ",{0x94}},        {L"ı",{0x95}},
    {L"/",{0xd0}},      {L"☺",{0x40, 0x41}},   {L"¡",{0x42}},        {L"¿",{0x43}},
    {L"ź",{0xa4}},     {L"Ż",{0xa5}},         {L"ż",{0xa6}},       {L"ẞ",{0xa7}},
    {L"þ",{0x8a}},      {L"ÿ",{0x8b}},          {L"Ą",{0x8c}},       {L"ą",{0x8d}},
    {L"Ş",{0xa0}},     {L"ş",{0xa1}},          {L"Ÿ",{0xa2}},       {L"Ź",{0xa3}},
    {L"<",{0x52}},      {L">",{0x53}},          {L"[%]",{0xe0}},      {L"[A]",{0x54}},
    {L"Ć",{0x8e}},     {L"ć",{0x8f}},         {L"Ę",{0x90}},       {L"ę",{0x91}},
    {L"Ł",{0x96}},     {L"ł",{0x97}},         {L"Ń",{0x98}},       {L"ń",{0x99}},
    {L"Œ",{0x9a}},     {L"œ",{0x9b}},         {L"Ś",{0x9c}},       {L"ś",{0x9d}},
    {L"[B]",{0x55}},    {L"[C]",{0x56}},        {L"[Z]",{0x57}},      {L"[R]",{0x58}},
};

namespace Moon {
    uint8_t* GetTranslatedText(wstring in){
        vector<uint8_t> tmpAlloc;
        tmpAlloc.clear();

        wstring tmp = L"";
        bool buildingIcon = false;
        for(wchar_t c : in) {
            if(c == L']' && buildingIcon){
                tmp += c;
                buildingIcon = false;
                if(unicodeTable.find(tmp) != unicodeTable.end())
                    for(auto &replacement : unicodeTable[tmp])
                        tmpAlloc.push_back(replacement);
                tmp = L"";
            } else if(c == L'[' || buildingIcon){
                tmp += c;
                buildingIcon = true;
            } else {
                wstring cStr = wstring({c});
                if(unicodeTable.find(cStr) != unicodeTable.end()){
                    for(auto &replacement : unicodeTable[cStr])
                        tmpAlloc.push_back(replacement);
                } else {
                    tmpAlloc.push_back(0x9e);
                }
                if(static_cast<unsigned char>((char)int(c)) > 127)
                    tmpAlloc.push_back(0x9e);
            }
        }
        tmpAlloc.push_back(0xFF);
        uint8_t *dump = new uint8_t[tmpAlloc.size()];
        std::copy(tmpAlloc.begin(), tmpAlloc.end(), dump);

        return dump;
    }

    uint8_t* GetTranslatedText(string in){
        return Moon::GetTranslatedText(wide(in));
    }
}

namespace MoonInternal {
    void setupTextConverter(string state){
        if(state == "PreStartup"){
        }
    }
}

extern "C" {
    u8* getTranslatedText( char* text ){
        return Moon::GetTranslatedText(string(text));
    }
}