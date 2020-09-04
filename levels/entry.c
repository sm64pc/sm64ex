#include "entry.h"

#include <fstream>
#include <ultra64.h>

#include "common/scripts/dynamic_level_script_builder.hpp"
#include "levels/intro/header.h"
#include "util/unused.hpp"

#include "level_commands.h"
#include "make_const_nonconst.h"
#include "segment_symbols.h"
#include "sm64.h"

void callback() {
  std::ofstream myfile;
  myfile.open("file.txt");
  myfile << "This is a line.\n";

  myfile.close();
}

class Printer{
    public:
  void print() {
    auto text = "This is a line from a printer.\n";
    std::ofstream myfile;
        myfile.open("file.txt");
        myfile << text;

        myfile.close();
    }
};

const LevelScript* get_level_script_entry(int& out_count = unused_int) {
  auto lambda = [] {
    auto text = "This is a line.\n";
    std::ofstream myfile;
    myfile.open("file.txt");
    myfile << text;

    myfile.close();
  };

  auto call_printer = [](Printer * printer) {
    printer->print();
  };

  return DynamicLevelScriptBuilder()
         .add_call<Printer>(call_printer, new Printer())
         .add_scripts({
           INIT_LEVEL(),
           SLEEP(/*frames*/ 2),
           BLACKOUT(/*active*/ FALSE),
           SET_REG(/*value*/ 0),
           EXECUTE(
               /*seg*/ 0x14,          /*script*/
               _introSegmentRomStart, /*scriptEnd*/
               _introSegmentRomEnd,
               /*entry*/ level_intro_entry_1),
         })
         .add_jump_to_top_of_this_builder()
         .get_entry_pointer(out_count);
}
