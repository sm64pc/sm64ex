#pragma once
#include "file_select.hpp"

#include <PR/ultratypes.h>

/**
 * Class representing the tile buttons on the main menu. This wraps around the
 * Object struct from the original game.
 */
class MenuButton {
public:
  explicit MenuButton(struct Object* impl) : impl(impl) {}

  void grow_from_main_menu();
  void shrink_to_main_menu();

  void grow_from_submenu();
  void shrink_to_submenu();

private:
  struct Object *impl;

  void set_state(MainMenuButtonStates state);
  MainMenuButtonStates get_state() const;

  void set_timer(s32 timer);
  s32 get_timer() const;

  /** Returns true once complete. */
  bool change_size_in_main_menu(bool grow);
  bool change_size_in_submenu(bool grow);
};
