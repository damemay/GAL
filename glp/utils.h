#pragma once

#define glp_diev(msg, ...) printf("glp: " msg "\n", __VA_ARGS__), exit(EXIT_FAILURE)
#define glp_die(msg) printf("glp: " msg "\n")
#define glp_logv(msg, ...) printf("glp: " msg "\n", __VA_ARGS__)
#define glp_log(msg) printf("glp: " msg "\n")
