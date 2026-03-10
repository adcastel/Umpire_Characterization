//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016-26, Lawrence Livermore National Security, LLC and Umpire
// project contributors. See the COPYRIGHT file for details.
//
// SPDX-License-Identifier: (MIT)
//////////////////////////////////////////////////////////////////////////////
#include <iostream>

#include "umpire/Allocator.hpp"
#include "umpire/ResourceManager.hpp"
#include "umpire/strategy/NumaPolicy.hpp"
#include "umpire/util/Macros.hpp"
#include "umpire/util/error.hpp"
#include "umpire/util/numa.hpp"

#if defined(UMPIRE_ENABLE_CUDA)
#include <cuda_runtime_api.h>
#endif

//TEST 0
void copy_data(void * source_data, size_t size, int host_dst)
{
  auto& rm = umpire::ResourceManager::getInstance();
  //auto host_nodes = umpire::numa::get_host_nodes();
  auto host_nodes = umpire::numa::get_allocatable_nodes();
  auto dest_allocator =
        rm.makeAllocator<umpire::strategy::NumaPolicy>("host_numa_dst_alloc", rm.getAllocator("HOST"), host_nodes[host_dst]);
  void* dest_data = dest_allocator.allocate(size);

  std::cout << "COPY Size (bytes); Time(s); BW(GBps)" << std::endl;
  // _sphinx_tag_tut_copy_start
  auto start = std::chrono::high_resolution_clock::now();
  rm.copy(dest_data, source_data);
  rm.memset(dest_data, 0);
  auto end = std::chrono::high_resolution_clock::now();
  // _sphinx_tag_tut_copy_end
  auto sec = std::chrono::duration<double>(end - start).count();
  //std::cout << size<<";"<< sec <<std::endl;
  std::cout << size<<";"<< sec<<";"<<(size/sec)/1e9 <<std::endl;

  dest_allocator.deallocate(dest_data);
}
//TEST 1
void move_data(void * source_data, size_t size, int host_dst)
{
  auto& rm = umpire::ResourceManager::getInstance();
  //auto host_nodes = umpire::numa::get_host_nodes();
  auto host_nodes = umpire::numa::get_allocatable_nodes();
  auto dest_allocator =
        rm.makeAllocator<umpire::strategy::NumaPolicy>("host_numa_dst_alloc", rm.getAllocator("HOST"), host_nodes[host_dst]);

  std::cout << "MOVE Size (bytes); Time(s); BW(GBps)" << std::endl;
  // _sphinx_tag_tut_copy_start
  auto start = std::chrono::high_resolution_clock::now();
  void * dest_data = rm.move(source_data, dest_allocator);
  rm.memset(dest_data, 0);
  auto end = std::chrono::high_resolution_clock::now();
  // _sphinx_tag_tut_copy_end
  auto sec = std::chrono::duration<double>(end - start).count();
  std::cout << size<<";"<< sec<<";"<<(size/sec)/1e9 <<std::endl;

  //rm.deallocate(dest_data);
}



int main(int argc, char *argv[])
{
  auto& rm = umpire::ResourceManager::getInstance();
  double sec;

  int TEST = (argc > 1) ? atol(argv[1]) : 0;

  // Get a list of the host NUMA nodes (e.g. one per socket)
  //auto host_nodes = umpire::numa::get_host_nodes();
  auto host_nodes = umpire::numa::get_allocatable_nodes();
  std::cout<<"NUMA Nodes: " << host_nodes.size() << std::endl;
  if (host_nodes.size() < 1) {
    UMPIRE_ERROR(umpire::runtime_error, "No NUMA nodes detected");
  }

  // Get a list of the host NUMA nodes (e.g. one per socket)
  auto allocatable_nodes = umpire::numa::get_allocatable_nodes();
  std::cout<<"NUMA Nodes: " << allocatable_nodes.size() << std::endl;
  if (allocatable_nodes.size() < 1) {
    UMPIRE_ERROR(umpire::runtime_error, "No NUMA allocatable nodes detected");
  }


  int host_src = (argc > 2) ? atoi(argv[2]) : 0;
  if (host_nodes.size() < host_src) {
      std::cout<<"Requested node "<< host_src << "as source but there are not as many nodes" <<std::endl;
      return 0;
  }
  int host_dst = (argc > 3) ? atoi(argv[3]) : 1;
  if (host_nodes.size() < host_dst) {
      std::cout<<"Requested node "<< host_dst << " as destination but there are not as many nodes" <<std::endl;
      return 0;
  }

  std::size_t SIZE = (argc > 4) ? atol(argv[4]) : 100; //Bytes

  std::cout<<"From " << host_src << " to " << host_dst <<std::endl;
  //size_t alloc_size = SIZE;
  size_t alloc_size = SIZE * umpire::get_page_size();

  // Create an allocator on the NUMA nodes
  auto host_src_alloc =
      rm.makeAllocator<umpire::strategy::NumaPolicy>("host_numa_src_alloc", rm.getAllocator("HOST"), host_nodes[host_src]);

  // Create an allocation on that node
  void* src_ptr = host_src_alloc.allocate(alloc_size);
  rm.memset(src_ptr, 0);
  if (TEST == 0) copy_data(src_ptr, alloc_size, host_dst);
  if (TEST == 1) move_data(src_ptr, alloc_size, host_dst);

  // Clean up by deallocating from the original allocator, since the
  // allocation record is still associated with that allocator
  host_src_alloc.deallocate(src_ptr);

  return 0;
}

