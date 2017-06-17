#include <iostream>

typedef int		i32;
typedef char	i8;

#ifdef __cplusplus
extern "C" {
#endif

i32 main(i32 argc, i8 **argv)
{
	std::cout << "Application CLI arguments:\r\n";
	for (i32 i = 0; i < argc; i++) {
		std::cout << "  arg " << std::dec << i << ": ";
		std::cout << argv[i] << "\r\n";
	}

	return 0;
}

#ifdef __cplusplus
}
#endif

