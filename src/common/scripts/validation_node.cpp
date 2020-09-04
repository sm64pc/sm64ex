#include "validation_node.hpp"

ValidationNode::~ValidationNode() { clear(); }


bool ValidationNode::is_valid() const { return is_valid_; }

void ValidationNode::validate() { is_valid_ = true; }

void ValidationNode::invalidate() {
  if (!is_valid_) { return; }

  is_valid_ = false;
  for (auto& parent : parents_) {
    if (!parent.expired()) { parent.lock()->invalidate(); }
  }
}


void ValidationNode::clear() {
  for (auto& child : children_) {
    if (!child.expired()) {
      child.lock()->parents_.erase(weak_from_this());
    }
  }
  children_.clear();

  invalidate();
}

void ValidationNode::depend_on(std::weak_ptr<ValidationNode> child) {
  children_.insert(child);
  child.lock()->parents_.insert(weak_from_this());

  invalidate();
}
