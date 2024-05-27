#ifndef FREE_LIST_HPP
#define FREE_LIST_HPP

#include <atomic>
#include <mutex>


class stack_t
{
public:
    struct node_t
    {
        node_t *next_ = nullptr;
    };

    node_t *head_;

public:

    stack_t() : head_(nullptr) {}

    void push(node_t *new_node)
    {
        new_node->next_ = head_;
        head_ = new_node;
    }

    node_t *pop()
    {
        node_t *top = head_;
        if( top )
          head_ = head_->next_;
        return top;
    }
};

class lock_free_stack_t
{
public:
    struct node_t
    {
        node_t *next_ = nullptr;
    };

    std::atomic<node_t *> head_;

public:

    lock_free_stack_t() : head_(nullptr) {}

    void push(node_t *new_node)
    {
        new_node->next_ = head_.load();
        while (!head_.compare_exchange_weak(new_node->next_, new_node));
    }

    node_t *pop()
    {
        node_t *top = head_.load();
        while (top && !head_.compare_exchange_weak(top, top->next_));
        return top;
    }
};

class lock_based_stack_t
{
public:
    struct node_t
    {
        node_t *next_ = nullptr;
    };

    node_t *head_;
    mutable std::mutex mutex_;

public:

    lock_based_stack_t() : head_(nullptr) {}

    void push(node_t *new_node)
    {
        const std::lock_guard<std::mutex> lock(mutex_);
        new_node->next_ = head_;
        head_ = new_node;
    }

    node_t *pop()
    {
        const std::lock_guard<std::mutex> lock(mutex_);
        node_t *top = head_;
        if( top )
          head_ = head_->next_;
        return top;
    }
};

#endif