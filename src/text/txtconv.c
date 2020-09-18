#include "txtconv.h"

struct Character charmap[356] = {
    {"0", {0x0, NULL}}, {"1", {0x1, NULL}}, {"{65297}", {0x1, NULL}}, {"2", {0x2, NULL}}, {"{65298}", {0x2, NULL}}, {"3", {0x3, NULL}}, {"{65299}", {0x3, NULL}}, {"4", {0x4, NULL}}, {"{65300}", {0x4, NULL}}, {"5", {0x5, NULL}},
    {"{65301}", {0x5, NULL}}, {"6", {0x6, NULL}}, {"{65302}", {0x6, NULL}}, {"7", {0x7, NULL}}, {"{65303}", {0x7, NULL}}, {"8", {0x8, NULL}}, {"{65304}", {0x8, NULL}}, {"9", {0x9, NULL}}, {"{65305}", {0x9, NULL}},
    {"A", {0xa, NULL}}, {"{65313}", {0xa, NULL}}, {"B", {0xb, NULL}}, {"{65314}", {0xb, NULL}}, {"C", {0xc, NULL}}, {"{65315}", {0xc, NULL}}, {"D", {0xd, NULL}}, {"{65316}", {0xd, NULL}}, {"E", {0xe, NULL}},
    {"{65317}", {0xe, NULL}}, {"F", {0xf, NULL}}, {"{65318}", {0xf, NULL}}, {"G", {0x10, NULL}}, {"{65319}", {0x10, NULL}}, {"H", {0x11, NULL}}, {"{65320}", {0x11, NULL}}, {"I", {0x12, NULL}}, {"{65321}", {0x12, NULL}},
    {"J", {0x13, NULL}}, {"{65322}", {0x13, NULL}}, {"K", {0x14, NULL}}, {"{65323}", {0x14, NULL}}, {"L", {0x15, NULL}}, {"{65324}", {0x15, NULL}}, {"M", {0x16, NULL}}, {"{65325}", {0x16, NULL}}, {"N", {0x17, NULL}},
    {"{65326}", {0x17, NULL}}, {"O", {0x18, NULL}}, {"{65327}", {0x18, NULL}}, {"P", {0x19, NULL}}, {"{65328}", {0x19, NULL}}, {"Q", {0x1a, NULL}}, {"{65329}", {0x1a, NULL}}, {"R", {0x1b, NULL}}, {"{65330}", {0x1b, NULL}},
    {"S", {0x1c, NULL}}, {"{65331}", {0x1c, NULL}}, {"T", {0x1d, NULL}}, {"{65332}", {0x1d, NULL}}, {"U", {0x1e, NULL}}, {"{65333}", {0x1e, NULL}}, {"V", {0x1f, NULL}}, {"{65334}", {0x1f, NULL}}, {"W", {0x20, NULL}},
    {"{65335}", {0x20, NULL}}, {"X", {0x21, NULL}}, {"{65336}", {0x21, NULL}}, {"Y", {0x22, NULL}}, {"{65337}", {0x22, NULL}}, {"Z", {0x23, NULL}}, {"{65338}", {0x23, NULL}}, {"a", {0x24, NULL}}, {"b", {0x25, NULL}},
    {"c", {0x26, NULL}}, {"d", {0x27, NULL}}, {"e", {0x28, NULL}}, {"f", {0x29, NULL}}, {"g", {0x2a, NULL}}, {"h", {0x2b, NULL}}, {"i", {0x2c, NULL}}, {"j", {0x2d, NULL}}, {"k", {0x2e, NULL}},
    {"l", {0x2f, NULL}}, {"m", {0x30, NULL}}, {"n", {0x31, NULL}}, {"o", {0x32, NULL}}, {"p", {0x33, NULL}}, {"q", {0x34, NULL}}, {"r", {0x35, NULL}}, {"s", {0x36, NULL}}, {"t", {0x37, NULL}},
    {"u", {0x38, NULL}}, {"v", {0x39, NULL}}, {"w", {0x3a, NULL}}, {"x", {0x3b, NULL}}, {"y", {0x3c, NULL}}, {"z", {0x3d, NULL}}, {"'", {0x3e, NULL}}, {".", {0x3f, NULL}}, {"{09786}", {0x40, 0x41}},
    {"{12354}", {0x40, NULL}}, {"{12356}", {0x41, NULL}}, {"{12358}", {0x42, NULL}}, {"{12360}", {0x43, NULL}}, {"{12362}", {0x44, NULL}}, {"{12363}", {0x45, NULL}}, {"{12365}", {0x46, NULL}}, {"{12367}", {0x47, NULL}}, {"{12369}", {0x48, NULL}},
    {"{12371}", {0x49, NULL}}, {"{12373}", {0x4a, NULL}}, {"{12375}", {0x4b, NULL}}, {"{12377}", {0x4c, NULL}}, {"{12379}", {0x4d, NULL}}, {"{12381}", {0x4e, NULL}}, {"{12383}", {0x4f, NULL}}, {"{12385}", {0x50, NULL}}, {"{12388}", {0x51, NULL}},
    {"{12390}", {0x52, NULL}}, {"{12392}", {0x53, NULL}}, {"{12394}", {0x54, NULL}}, {"{12395}", {0x55, NULL}}, {"{12396}", {0x56, NULL}}, {"{12397}", {0x57, NULL}}, {"{12398}", {0x58, NULL}}, {"{12399}", {0x59, NULL}}, {"{12402}", {0x5a, NULL}},
    {"{12405}", {0x5b, NULL}}, {"{12408}", {0x5c, NULL}}, {"{12411}", {0x5d, NULL}}, {"{12414}", {0x5e, NULL}}, {"{12415}", {0x5f, NULL}}, {"{12416}", {0x60, NULL}}, {"{12417}", {0x61, NULL}}, {"{12418}", {0x62, NULL}}, {"{12420}", {0x63, NULL}},
    {"{12422}", {0x64, NULL}}, {"{12424}", {0x65, NULL}}, {"{12425}", {0x66, NULL}}, {"{12426}", {0x67, NULL}}, {"{12427}", {0x68, NULL}}, {"{12428}", {0x69, NULL}}, {"{12429}", {0x6a, NULL}}, {"{12431}", {0x6b, NULL}}, {"{12434}", {0x6c, NULL}},
    {"{12435}", {0x6d, NULL}}, {"{12290}", {0x6e, NULL}}, {",", {0x6f, NULL}}, {"{12289}", {0x6f, NULL}}, {"{12450}", {0x70, NULL}}, {"{12452}", {0x71, NULL}}, {"{12454}", {0x72, NULL}}, {"{12456}", {0x73, NULL}}, {"{12458}", {0x74, NULL}},
    {"{12459}", {0x75, NULL}}, {"{12461}", {0x76, NULL}}, {"{12463}", {0x77, NULL}}, {"{12465}", {0x78, NULL}}, {"{12467}", {0x79, NULL}}, {"{12469}", {0x7a, NULL}}, {"{12471}", {0x7b, NULL}}, {"{12473}", {0x7c, NULL}}, {"{12475}", {0x7d, NULL}},
    {"{12477}", {0x7e, NULL}}, {"{12479}", {0x7f, NULL}}, {"{12481}", {0x80, NULL}}, {"{12484}", {0x81, NULL}}, {"{12486}", {0x82, NULL}}, {"{12488}", {0x83, NULL}}, {"{12490}", {0x84, NULL}}, {"{12491}", {0x85, NULL}}, {"{12492}", {0x86, NULL}},
    {"{12493}", {0x87, NULL}}, {"{12494}", {0x88, NULL}}, {"{12495}", {0x89, NULL}}, {"{12498}", {0x8a, NULL}}, {"{12501}", {0x8b, NULL}}, {"{12504}", {0x8c, NULL}}, {"{12507}", {0x8d, NULL}}, {"{12510}", {0x8e, NULL}}, {"{12511}", {0x8f, NULL}},
    {"{12512}", {0x90, NULL}}, {"{12513}", {0x91, NULL}}, {"{12514}", {0x92, NULL}}, {"{12516}", {0x93, NULL}}, {"{12518}", {0x94, NULL}}, {"{12520}", {0x95, NULL}}, {"{12521}", {0x96, NULL}}, {"{12522}", {0x97, NULL}}, {"{12523}", {0x98, NULL}},
    {"{12524}", {0x99, NULL}}, {"{12525}", {0x9a, NULL}}, {"{12527}", {0x9b, NULL}}, {"{12530}", {0x9c, NULL}}, {"{12531}", {0x9d, NULL}}, {" ", {0x9e, NULL}}, {"{12288}", {0x9e, NULL}}, {"-", {0x9f, NULL}}, {"{12540}", {0x9f, NULL}},
    {"{12359}", {0xa0, NULL}}, {"{12387}", {0xa1, NULL}}, {"{12419}", {0xa2, NULL}}, {"{12421}", {0xa3, NULL}}, {"{12423}", {0xa4, NULL}}, {"{12353}", {0xa5, NULL}}, {"{12355}", {0xa6, NULL}}, {"{12357}", {0xa7, NULL}}, {"{12361}", {0xa8, NULL}},
    {"{12455}", {0xd0, NULL}}, {"{12483}", {0xd1, NULL}}, {"{12515}", {0xd2, NULL}}, {"{12517}", {0xd3, NULL}}, {"{12519}", {0xd4, NULL}}, {"{12449}", {0xd5, NULL}}, {"{12451}", {0xd6, NULL}}, {"{12453}", {0xd7, NULL}}, {"{12457}", {0xd8, NULL}},
    {"[%]", {0xe0, NULL}}, {"(", {0xe1, NULL}}, {"{65288}", {0xe1, NULL}}, {")(", {0xe2, NULL}}, {"）（", {0xe2, NULL}}, {")", {0xe3, NULL}}, {"{65289}", {0xe3, NULL}}, {"+", {0xe4, NULL}}, {"{08596}", {0xe4, NULL}},
    {"&", {0xe5, NULL}}, {":", {0xe6, NULL}}, {"{12443}", {0xf0, NULL}}, {"{12444}", {0xf1, NULL}}, {"!", {0xf2, NULL}}, {"{65281}", {0xf2, NULL}}, {"%", {0xf3, NULL}}, {"{65285}", {0xf3, NULL}}, {"?", {0xf4, NULL}},
    {"{65311}", {0xf4, NULL}}, {"{12302}", {0xf5, NULL}}, {"{12303}", {0xf6, NULL}}, {"~", {0xf7, NULL}}, {"{65374}", {0xf7, NULL}}, {"{08230}", {0xf8, NULL}}, {"$", {0xf9, NULL}}, {"{09733}", {0xfa, NULL}}, {"{00215}", {0xfb, NULL}},
    {"{12539}", {0xfc, NULL}}, {"{09734}", {0xfd, NULL}}, {"\n", {0xfe, NULL}}, {"{12364}", {0xf0, 0xf0}}, {"{12366}", {0xf0, 0xf0}}, {"{12368}", {0xf0, 0xf0}}, {"{12370}", {0xf0, 0xf0}}, {"{12372}", {0xf0, 0xf0}}, {"{12374}", {0xf0, 0xf0}},
    {"{12376}", {0xf0, 0xf0}}, {"{12378}", {0xf0, 0xf0}}, {"{12380}", {0xf0, 0xf0}}, {"{12382}", {0xf0, 0xf0}}, {"{12384}", {0xf0, 0xf0}}, {"{12386}", {0xf0, 0xf0}}, {"{12389}", {0xf0, 0xf0}}, {"{12391}", {0xf0, 0xf0}}, {"{12393}", {0xf0, 0xf0}},
    {"{12400}", {0xf0, 0xf0}}, {"{12403}", {0xf0, 0xf0}}, {"{12406}", {0xf0, 0xf0}}, {"{12409}", {0xf0, 0xf0}}, {"{12412}", {0xf0, 0xf0}}, {"{12460}", {0xf0, 0xf0}}, {"{12462}", {0xf0, 0xf0}}, {"{12464}", {0xf0, 0xf0}}, {"{12466}", {0xf0, 0xf0}},
    {"{12468}", {0xf0, 0xf0}}, {"{12470}", {0xf0, 0xf0}}, {"{12472}", {0xf0, 0xf0}}, {"{12474}", {0xf0, 0xf0}}, {"{12476}", {0xf0, 0xf0}}, {"{12478}", {0xf0, 0xf0}}, {"{12480}", {0xf0, 0xf0}}, {"{12482}", {0xf0, 0xf0}}, {"{12485}", {0xf0, 0xf0}},
    {"{12487}", {0xf0, 0xf0}}, {"{12489}", {0xf0, 0xf0}}, {"{12496}", {0xf0, 0xf0}}, {"{12499}", {0xf0, 0xf0}}, {"{12502}", {0xf0, 0xf0}}, {"{12505}", {0xf0, 0xf0}}, {"{12508}", {0xf0, 0xf0}}, {"{12401}", {0xf1, 0xf1}}, {"{12404}", {0xf1, 0xf1}},
    {"{12407}", {0xf1, 0xf1}}, {"{12410}", {0xf1, 0xf1}}, {"{12413}", {0xf1, 0xf1}}, {"{12497}", {0xf1, 0xf1}}, {"{12500}", {0xf1, 0xf1}}, {"{12503}", {0xf1, 0xf1}}, {"{12506}", {0xf1, 0xf1}}, {"{12509}", {0xf1, 0xf1}}, {"^", {0x50, NULL}},
    {"|", {0x51, NULL}}, {"<", {0x52, NULL}}, {">", {0x53, NULL}}, {"[A]", {0x54, NULL}}, {"[B]", {0x55, NULL}}, {"[C]", {0x56, NULL}}, {"[Z]", {0x57, NULL}}, {"[R]", {0x58, NULL}}, {"/", {0xd0, NULL}},
    {"the", {0xd1, NULL}}, {"you", {0xd2, NULL}}, {"{00224}", {0x60, NULL}}, {"{00226}", {0x61, NULL}}, {"{00228}", {0x62, NULL}}, {"{00192}", {0x64, NULL}}, {"{00194}", {0x65, NULL}}, {"{00196}", {0x66, NULL}}, {"{00232}", {0x70, NULL}},
    {"{00234}", {0x71, NULL}}, {"{00235}", {0x72, NULL}}, {"{00200}", {0x74, NULL}}, {"{00202}", {0x75, NULL}}, {"{00203}", {0x76, NULL}}, {"{00249}", {0x80, NULL}}, {"{00251}", {0x81, NULL}},
    {"{00217}", {0x84, NULL}}, {"{00219}", {0x85, NULL}}, {"{00244}", {0x91, NULL}}, {"{00246}", {0x92, NULL}}, {"{00212}", {0x95, NULL}}, {"{00214}", {0x96, NULL}}, {"{00238}", {0xa1, NULL}},
    {"{00239}", {0xa2, NULL}}, {"{00223}", {0xec, NULL}}, {"{00199}", {0xed, NULL}}, {"{00231}", {0xee, NULL}}, {"{08222}", {0xf0, NULL}},

    // SPECIAL CHARACTERS
    {"{00225}", {112, NULL}},
    {"{00233}", {113, NULL}},
    {"{00237}", {114, NULL}},
    {"{00243}", {115, NULL}},
    {"{00250}", {116, NULL}},
    {"{00193}", {117, NULL}},
    {"{00201}", {118, NULL}},
    {"{00205}", {119, NULL}},
    {"{00211}", {120, NULL}},
    {"{00218}", {121, NULL}},
    {"{00241}", {122, NULL}},
    {"{00209}", {123, NULL}},
    {"{00252}", {124, NULL}},
    {"{00220}", {125, NULL}},
    {"{00191}", {126, NULL}},
    {"{00161}", {127, NULL}}
};

struct Character getCharacter(char* ch){
    struct Character tmp = {NULL, {NULL, NULL}};
    for(s32 cmid = 0; cmid < sizeof(charmap) / sizeof(struct Character); cmid++){
        if(charmap[cmid].txt != NULL){
            if(strncmp(charmap[cmid].txt, ch, strlen(charmap[cmid].txt)) == 0) {
                tmp = charmap[cmid];
                break;
            }
        }
    }
    return tmp;
}

u8 * getTranslatedText(char * txt){

    txt = (txt == NULL ? "" : txt);

    s32 cid;
    s32 strSize = strlen(txt);
    u8 * tmp = malloc((strSize + 1) * sizeof(u8));
    u8 icon = FALSE;

    s32 shiftArr = 0;

    char tmpId = 0;
    char tmpIcon[3];

    char tmpSpecialChar[7];

    s32 ignoreUntil = 0;
    for(cid = 0; cid < strSize; cid++){
        char ch = txt[cid];

        if(ch == '['){
            tmpIcon[0] = ch;
            tmpIcon[1] = txt[cid + 1];
            tmpIcon[2] = txt[cid + 2];
            struct Character ctm = getCharacter(tmpIcon);
            if(ctm.txt != NULL){
                shiftArr += 2;
                cid += 2;
                for(int cl = 0; cl < 2; cl++){
                    if(ctm.value[cl] != NULL){
                        tmp[cid - shiftArr + cl] = ctm.value[cl];
                        shiftArr-=cl;
                    }
                }
                memset(tmpIcon, 0, sizeof(tmpIcon));
            }
        } else if(ch == '{'){
            tmpSpecialChar[0] = ch;
            tmpSpecialChar[1] = txt[cid + 1];
            tmpSpecialChar[2] = txt[cid + 2];
            tmpSpecialChar[3] = txt[cid + 3];
            tmpSpecialChar[4] = txt[cid + 4];
            tmpSpecialChar[5] = txt[cid + 5];
            tmpSpecialChar[6] = txt[cid + 6];

            struct Character ctm = getCharacter(tmpSpecialChar);

            if(ctm.txt != NULL){
                shiftArr += 5;
                cid += 5;
                for(int cl = 0; cl < 2; cl++){
                    if(ctm.value[cl] != NULL){
                        tmp[cid - shiftArr + cl] = ctm.value[cl];
                        shiftArr-=cl;
                    }
                }
                memset(tmpSpecialChar, 0, sizeof(tmpSpecialChar));
            }
        } else {
            char findTxt[1] = {ch};

            struct Character ctm = getCharacter(findTxt);
            if(ctm.txt != NULL){
                tmp[cid - shiftArr] = ctm.value[0];
            }else{
                tmp[cid - shiftArr] = 0x9E;
            }
        }
    }

    tmp = realloc(tmp, (strSize - shiftArr + 1) * sizeof(u8));
    tmp[strSize - shiftArr] = 0xFF;

    return tmp;
}