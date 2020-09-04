#pragma once
#include <memory>
#include <set>

class ValidationNode;

typedef std::owner_less<std::weak_ptr<ValidationNode>> ValidationNodeComparer;

class ValidationNode : public std::enable_shared_from_this<ValidationNode> {
public:
  ValidationNode() = default;
  ValidationNode(const ValidationNode& other) = delete;
  ~ValidationNode();

  bool is_valid() const;
  void validate();
  void invalidate();

  void clear();
  void depend_on(std::weak_ptr<ValidationNode> child);

private:
  std::set<std::weak_ptr<ValidationNode>, ValidationNodeComparer> parents_;
  std::set<std::weak_ptr<ValidationNode>, ValidationNodeComparer> children_;

  bool is_valid_ = false;
};
