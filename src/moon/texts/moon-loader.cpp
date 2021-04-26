#include "moon-loader.h"
#include <iostream>

#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <dirent.h>

extern "C" {
#include "text/text-loader.h"
#include "pc/platform.h"
#include "pc/configfile.h"
#include "text/libs/io_utils.h"
}

using namespace std;
using namespace rapidjson;

typedef GenericDocument<UTF16<>> WDocument;
typedef GenericValue<UTF16<>> WValue;

vector<LanguageEntry*> languages;

LanguageEntry *current;

#define SECRET_NULL 24

void Moon_LoadLanguage( string path ) {

    LanguageEntry * language = new LanguageEntry(); 
    FILE* fp = fopen(path.c_str(), "r");
    char readBuffer[65536];
    FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    AutoUTFInputStream<unsigned, FileReadStream> eis(is);

    WDocument raw;
    raw.ParseStream<0, AutoUTF<unsigned>>(eis);

    WValue manifest, dialogs, courses, secrets, options, strings;
    
    manifest = raw[L"manifest"];
    dialogs  = raw[L"dialogs"];
    courses  = raw[L"courses"];
    secrets  = raw[L"secrets"];
    options  = raw[L"options"];
    strings  = raw[L"strings"];
    
    language->name = narrow(manifest[L"langName"].GetString());
    language->logo = narrow(manifest[L"langLogo"].GetString());

    for (WValue& dialog : dialogs.GetArray()) {        
        int linesPerBox = dialog[L"linesPerBox"].GetInt();
        int leftOffset  = dialog[L"leftOffset"].GetInt();
        int width       = dialog[L"width"].GetInt();
        int id          = dialog[L"ID"].GetInt();

        DialogEntry *entry = new DialogEntry();
        entry->linesPerBox = linesPerBox;
        entry->leftOffset  = leftOffset;
        entry->width       = width;
        entry->unused      = 1;

        wstring base;

        for (WValue& line : dialog[L"lines"].GetArray()){
            base += line.GetString();
            base += L"\n";
        }
        
        entry->str         = getTranslatedText(narrow(base).c_str());
        language->dialogs.push_back(entry);
    }
    
    int course_name_table_size = courses.Size() + secrets.Size();    
    int courseId = 0;
    int padding = 0;

    u8* tmpCourses[course_name_table_size];

    for (WValue& course : courses.GetArray()){
        
        if(courseId + 1 <= courses.Size() - 1) {
            tmpCourses[courseId] = getTranslatedText(narrow(course[L"course"].GetString()).c_str());
            courseId++;
        }

        for (WValue& act : course[L"acts"].GetArray()){
            language->acts.push_back(getTranslatedText(narrow(act.GetString()).c_str()));
        }   
    }

    for (WValue& secret : secrets.GetArray()){
        if(courseId == SECRET_NULL) {
            tmpCourses[courseId] = getTranslatedText(0);
            padding++;
        }

        tmpCourses[courseId + padding] = getTranslatedText(narrow(secret.GetString()).c_str());
        courseId++;
    }

    language->courses.insert(language->courses.end(), &tmpCourses[0], &tmpCourses[course_name_table_size]);

    for (WValue::ConstMemberIterator option = options.MemberBegin(); option != options.MemberEnd(); ++option) {
        language->strings.insert(pair<string, u8*>(
            narrow(option->name.GetString()), 
            getTranslatedText(narrow(option->value.GetString()).c_str())
        ));
    }

    for (WValue::ConstMemberIterator item = strings.MemberBegin(); item != strings.MemberEnd(); ++item) {
        language->strings.insert(pair<string, u8*>(
            narrow(item->name.GetString()), 
            getTranslatedText(narrow(item->value.GetString()).c_str())
        ));
    }    

    languages.push_back(language);
    languagesAmount = languages.size();
}

u8 *Moon_GetKey(string key) {
    return current->strings[key];
}

void Moon_InitLanguages( char *exePath, char *gamedir ) {    
    
    string l_path(exePath);

    string languages_dir = l_path.substr(0, l_path.find_last_of("/\\")) + "/" + gamedir + "/texts/";
    printf("Loading Directory: %s\n", languages_dir.c_str());

    // Scan directory for JSON files
    DIR *dir = opendir(languages_dir.c_str());
    if (dir) {
        struct dirent *de;
        while ((de = readdir(dir)) != NULL) {
            const char *extension = get_filename_ext(de->d_name);
            if (strcmp(extension, "json") == 0) {
                string file = languages_dir + de->d_name;
                printf("Loading File: %s\n", file.c_str());
                Moon_LoadLanguage( file );
            }
        }
        closedir(dir);
    }

    Moon_SetLanguage(languages[configLanguage]);    
}

void Moon_SetLanguage(LanguageEntry *new_language) {
    current = new_language;
    dialogPool = current->dialogs.data();
    seg2_act_name_table = current->acts.data();
    seg2_course_name_table = current->courses.data();
}