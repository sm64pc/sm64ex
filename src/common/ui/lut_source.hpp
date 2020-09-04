#pragma once

/**
 * This changes the font used when text rendering, since each one has different
 * glyphs.
 */
enum class LutSource {
  UNDEFINED,
  DEFAULT,
  JAPANESE,
  DIFF,
};