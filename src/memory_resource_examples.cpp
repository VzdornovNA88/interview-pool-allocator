#include <iostream>
#include <thread>

#include <memory_resource_user.hpp>

int main()
{
   mallocfree_memory_resource_t heap_memory; 
   pool_memory_resource_t<> mem_pool(&heap_memory,4,sizeof(int));  

   void *ptr = mem_pool.allocate(sizeof(int));
   std::cout<<"ptr = "<<ptr<<std::endl;
   auto iP333 = new(ptr) int (333);
   std::cout<<"iP333 = "<<*iP333<<std::endl;

   void *ptr1 = mem_pool.allocate(sizeof(int));
   std::cout<<"ptr1 = "<<ptr1<<std::endl;

   mem_pool.deallocate(ptr,sizeof(int));

   ptr = mem_pool.allocate(sizeof(int));
   std::cout<<"ptr = "<<ptr<<std::endl;
   auto iP777 = new(ptr) int (777);
   std::cout<<"iP777 = "<<*iP777<<std::endl;


   //////////////////////////////////////////

int cnt = 16;
   using test_traits1_t = memmory_resource_traits_t<multithreading_policy_t::LK_BASED>;
   pool_memory_resource_t<test_traits1_t> lock_free_mem_pool(&heap_memory,cnt,sizeof(int));

   auto* pool_ptr = &lock_free_mem_pool;

   std::thread memory_consumer1([pool_ptr,cnt]() mutable {
    int i = cnt/2;
    while (i-->0) {
        auto ptr = pool_ptr->allocate(sizeof(int));
        std::cout<<"memory_consumer1 ptr  = "<<ptr<<std::endl;
    }
   });


    std::thread memory_consumer2([pool_ptr,cnt]() mutable {
    int i = cnt/2;
    while (i-->0) {
        auto ptr = pool_ptr->allocate(sizeof(int));
        std::cout<<"memory_consumer2 ptr  = "<<ptr<<std::endl;
    }
   });

   memory_consumer1.join();
   memory_consumer2.join();
}