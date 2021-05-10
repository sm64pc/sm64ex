#include "moon-loader.h"
#include <iostream>

#include "moon/utils/moon-env.h"

#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <dirent.h>
#include <algorithm>

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

#define SECRET_NULL 24

namespace Moon {

    vector<LanguageEntry*> languages;
    LanguageEntry *current;

    void loadLanguage( string path ) {

        FILE* fp = fopen(path.c_str(), "r");
        char readBuffer[65536];
        FileReadStream is(fp, readBuffer, sizeof(readBuffer));
        AutoUTFInputStream<unsigned, FileReadStream> eis(is);

        WDocument raw;
        raw.ParseStream<0, AutoUTF<unsigned>>(eis);

        WValue manifest, dialogs, courses, secrets, options, strings;

        if(!raw.HasMember(L"manifest")) {
            cout << "Failed to load language" << endl;
            cout << "Missing manifest" << endl;
            return;
        }

        manifest = raw[L"manifest"];

        bool isChild = manifest.HasMember(L"parent");

        LanguageEntry * language;

        if(!isChild){
            language = new LanguageEntry();
        } else {
            isChild = false;
            string parentName = narrow(manifest[L"langLogo"].GetString());
            for(auto &lng : languages){
                if(lng->name == parentName){
                    language = lng;
                    isChild = true;
                    break;
                }
            }
        }

        if(!isChild){
            language->name = narrow(manifest[L"langName"].GetString());
            language->logo = narrow(manifest[L"langLogo"].GetString());
        }

        if(raw.HasMember(L"dialogs")){
            dialogs = raw[L"dialogs"];
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
        }

        if(raw.HasMember(L"courses") && raw.HasMember(L"secrets")){
            courses = raw[L"courses"];
            secrets = raw[L"secrets"];
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
        }

        if(raw.HasMember(L"options")){
            options = raw[L"options"];
            for (WValue::ConstMemberIterator option = options.MemberBegin(); option != options.MemberEnd(); ++option) {
                language->strings.insert(pair<string, string>(
                    narrow(option->name.GetString()),
                    narrow(option->value.GetString())
                ));
            }
        }

        if(raw.HasMember(L"secrets")){
            strings = raw[L"strings"];
            for (WValue::ConstMemberIterator item = strings.MemberBegin(); item != strings.MemberEnd(); ++item) {
                language->strings.insert(pair<string, string>(
                    narrow(item->name.GetString()),
                    narrow(item->value.GetString())
                ));
            }
        }

        if(!isChild) languages.push_back(language);
        languagesAmount = languages.size();
    }

    void setCurrentLanguage(LanguageEntry *new_language) {
        current = new_language;
        dialogPool = current->dialogs.data();
        seg2_act_name_table = current->acts.data();
        seg2_course_name_table = current->courses.data();
    }

    string getLanguageKey(string key){
        return current->strings[key];
    }
}

namespace MoonInternal {

    void scanLanguagesDirectory(){
        string cwd     = MoonInternal::getEnvironmentVar("MOON_CWD");
        string gameDir = MoonInternal::getEnvironmentVar("ASSETS_DIR");

        string languagesDir = cwd.substr(0, cwd.find_last_of("/\\")) + "/" + gameDir + "/texts/";

        // Scan directory for JSON files
        DIR *dir = opendir(languagesDir.c_str());
        if (dir) {
            struct dirent *de;
            while ((de = readdir(dir)) != NULL) {
                const char *extension = get_filename_ext(de->d_name);
                if (strcmp(extension, "json") == 0) {
                    string file = languagesDir + de->d_name;
                    Moon::loadLanguage( file );
                }
            }
            closedir(dir);
        }
    }

    void setupLanguageEngine( string state ){
        if(state == "PreInit"){
            MoonInternal::scanLanguagesDirectory();
            return;
        }
        if(state == "Init"){
            Moon::setCurrentLanguage(Moon::languages[configLanguage]);
            return;
        }
    }
}