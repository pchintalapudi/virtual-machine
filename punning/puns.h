#ifndef PUNNING_PUNS_H
#define PUNNING_PUNS_H

//haha very punny
#define PUN(type, name, from) type name; std::memcpy(&name, from, sizeof(type))
#endif