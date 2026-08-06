add_executable(example_benchmark)

target_link_libraries(example_benchmark PUBLIC clbench)

target_sources(example_benchmark
	PRIVATE	
        example_benchmark.c 
)

clb_add_benchmark(example_benchmark)
