#include "menu_button.hpp"

#include "object_fields.h"

/**
 * Public
 */
void MenuButton::grow_from_main_menu() {
  if (change_size_in_main_menu(true)) {
    set_state(MENU_BUTTON_STATE_FULLSCREEN);
  }
}

void MenuButton::shrink_to_main_menu() {
  if (change_size_in_main_menu(false)) {
    set_state(MENU_BUTTON_STATE_DEFAULT);
  }
}

void MenuButton::grow_from_submenu() {
  if (change_size_in_submenu(true)) {
    set_state(MENU_BUTTON_STATE_FULLSCREEN);
  }
}

void MenuButton::shrink_to_submenu() {
  if (change_size_in_submenu(false)) {
    set_state(MENU_BUTTON_STATE_DEFAULT);
  }
}


/**
 * Private
 */
void MenuButton::set_timer(s32 timer) { impl->oMenuButtonTimer = timer; }
s32 MenuButton::get_timer() const { return impl->oMenuButtonTimer; }

void MenuButton::set_state(MainMenuButtonStates state) {
  impl->oMenuButtonState = state;
}

MainMenuButtonStates MenuButton::get_state() const {
  return (MainMenuButtonStates) impl->oMenuButtonState;
}

bool MenuButton::change_size_in_main_menu(bool grow) {
  const auto timer = get_timer();
  const auto growthSign = grow ? 1 : -1;

  if (timer < 16) { impl->oFaceAngleYaw += growthSign * 0x800; }
  if (timer < 8) { impl->oFaceAnglePitch += growthSign * 0x800; }
  if (timer >= 8 && timer < 16) { impl->oFaceAnglePitch -= growthSign * 0x800; }

  impl->oParentRelativePosX -= growthSign * impl->oMenuButtonOrigPosX / 16.0;
  impl->oParentRelativePosY -= growthSign * impl->oMenuButtonOrigPosY / 16.0;

  if ((grow && impl->oPosZ < impl->oMenuButtonOrigPosZ + 17800)
      || (!grow && impl->oPosZ > impl->oMenuButtonOrigPosZ)) {
    impl->oParentRelativePosZ += growthSign * 1112.5;
  }

  set_timer(timer + 1);
  if (get_timer() == 16) {
    impl->oParentRelativePosX = grow ? 0 : impl->oMenuButtonOrigPosX;
    impl->oParentRelativePosY = grow ? 0 : impl->oMenuButtonOrigPosY;

    set_timer(0);

    return true;
  }

  return false;
}

bool MenuButton::change_size_in_submenu(bool grow) {
  const auto timer = get_timer();
  const auto growthSign = grow ? 1 : -1;

  if (timer < 16) { impl->oFaceAngleYaw += growthSign * 0x800; }
  if (timer < 8) { impl->oFaceAnglePitch += growthSign * 0x800; }
  if (timer >= 8 && timer < 16) { impl->oFaceAnglePitch -= growthSign * 0x800; }

  impl->oParentRelativePosX -= growthSign * impl->oMenuButtonOrigPosX / 16.0;
  impl->oParentRelativePosY -= growthSign * impl->oMenuButtonOrigPosY / 16.0;

  if (grow || (!grow && impl->oPosZ > impl->oMenuButtonOrigPosZ)) {
    impl->oParentRelativePosZ -= growthSign * 116.25;
  }

  set_timer(timer + 1);
  if (get_timer() == 16) {
    impl->oParentRelativePosX = grow ? 0 : impl->oMenuButtonOrigPosX;
    impl->oParentRelativePosY = grow ? 0 : impl->oMenuButtonOrigPosY;
    set_timer(0);

    return true;
  }

  return false;
}
