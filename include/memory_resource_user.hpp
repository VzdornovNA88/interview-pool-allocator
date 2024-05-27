#ifndef MEMORY_RESUORCE_USER_HPP
#define MEMORY_RESUORCE_USER_HPP

#include <cstddef>
#include <algorithm>
#include <type_traits>

#include <free_list.hpp>

class memory_resource_t
{
public:
    virtual ~memory_resource_t() noexcept = default;

    void *allocate(const size_t bytes, const size_t align = alignof(max_align_t))
    {
        return do_allocate(bytes, align);
    }

    void deallocate(void *const ptr, const size_t bytes, const size_t align = alignof(max_align_t))
    {
        return do_deallocate(ptr, bytes, align);
    }

private:
    virtual void *do_allocate(size_t bytes, size_t align) = 0;
    virtual void do_deallocate(void *ptr, size_t bytes, size_t align) = 0;
};

enum class multithreading_policy_t : char
{
    LK_NONE = 0,
    LK_FREE = 1,
    LK_BASED = 2,
};

template <multithreading_policy_t MT_POLICY>
struct memmory_resource_traits_t
{

    template <bool B, class T, class F>
    using user_conditional_t = typename std::conditional<B, T, F>::type;

    using free_list_t = user_conditional_t<MT_POLICY == multithreading_policy_t::LK_NONE, stack_t,
                                           user_conditional_t<MT_POLICY == multithreading_policy_t::LK_FREE, lock_free_stack_t,
                                                              user_conditional_t<MT_POLICY == multithreading_policy_t::LK_BASED, lock_based_stack_t,
                                                                                 void>>>;

    static std::size_t get_min_chunk_size() {
        return sizeof(typename free_list_t::node_t);
    }

    static void release_memory_to(free_list_t &free_list, void *ptr)
    {
        free_list.push(::new (ptr) typename free_list_t::node_t);
    }

    static void *acquire_memory_from(free_list_t &free_list)
    {
        return static_cast<void *>(free_list.pop());
    }
};

class mallocfree_memory_resource_t : public memory_resource_t
{
    virtual void *do_allocate(size_t bytes, size_t align) override
    {
        return malloc(bytes);
    };

    virtual void do_deallocate(void *ptr, size_t bytes, size_t align) override
    {
        free(ptr);
    };

public:
    virtual ~mallocfree_memory_resource_t() noexcept {}
};

template <typename TRAITS = memmory_resource_traits_t<multithreading_policy_t::LK_NONE>>
class pool_memory_resource_t : public memory_resource_t
{
    typename TRAITS::free_list_t free_list_;

    memory_resource_t *upstream_;
    void *begin_;
    std::size_t chunk_size_;
    std::size_t capacity_;

    virtual void *do_allocate(size_t bytes, size_t align) override
    {
        // assert(bytes == chunk_size_ && "Allocation size must be equal to chunk size");
        return TRAITS::acquire_memory_from(free_list_);
    };

    virtual void do_deallocate(void *ptr, size_t bytes, size_t align) override
    {
        // assert(bytes == chunk_size_ && "Allocation size must be equal to chunk size");
        TRAITS::release_memory_to(free_list_, ptr);
    };

public:
    pool_memory_resource_t(memory_resource_t *upstream, const std::size_t chunks_amount, const std::size_t chunk_size) : upstream_(upstream), begin_(nullptr), chunk_size_(chunk_size), capacity_(chunks_amount * std::max(chunk_size_, TRAITS::get_min_chunk_size()))
    {
        begin_ = upstream_->allocate(capacity_);

        for (int i = 0; i < chunks_amount; ++i)
        {
            auto ptr = static_cast<void *>(static_cast<char *>(begin_) + i * std::max(chunk_size_, TRAITS::get_min_chunk_size()));

            //! check alignment

            TRAITS::release_memory_to(free_list_, ptr);
        }
    }

    virtual ~pool_memory_resource_t() noexcept
    {
        upstream_->deallocate(begin_, capacity_);
    }
};

#endif