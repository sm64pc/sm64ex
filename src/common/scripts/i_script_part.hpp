#pragma once

#include <memory>
#include <vector>

#include "util.hpp"

template <typename TScript>
class IScriptBuilder;

template <typename TScript>
class IScriptPart {
 public:
  IScriptPart() {}
  IScriptPart(const IScriptPart& other) = delete;

  virtual int size() const = 0;
  virtual void build_into(TScript* dst, int& dst_pos) const = 0;
};

template <typename TScript>
class SingleScriptPart : public IScriptPart<TScript> {
 public:
  SingleScriptPart(TScript script) : script_(script) {}

  SingleScriptPart(const SingleScriptPart& other) = delete;

  int size() const override { return 1; }

  void build_into(TScript* dst, int& dst_pos) const override {
    append_script(dst, dst_pos, script_);
  }

 private:
  TScript script_;
};

template <typename TScript>
class MultipleScriptPart : public IScriptPart<TScript> {
 public:
  MultipleScriptPart(const TScript* scripts, int script_count) {
    for (auto i = 0; i < script_count; ++i) {
      scripts_.push_back(scripts[i]);
    }
  }

  MultipleScriptPart(std::initializer_list<const TScript> scripts) {
    for (auto script : scripts) {
      scripts_.push_back(script);
    }
  }

  MultipleScriptPart(const MultipleScriptPart& other) = delete;

  int size() const override { return scripts_.size(); }

  void build_into(TScript* dst, int& dst_pos) const override {
    append_scripts(dst, dst_pos, &scripts_[0], scripts_.size());
  }

 private:
  std::vector<TScript> scripts_;
};

template <typename TScript>
class BuilderScriptPart : public IScriptPart<TScript> {
 public:
  BuilderScriptPart(std::shared_ptr<IScriptBuilder<TScript>> builder)
      : builder_(std::move(builder)) {}

  BuilderScriptPart(const BuilderScriptPart& other) = delete;

  int size() const override { return builder_->size(); }

  void build_into(TScript* dst, int& dst_pos) const override {
    builder_->build_into(dst, dst_pos);
  }

 private:
  std::shared_ptr<IScriptBuilder<TScript>> builder_;
};
