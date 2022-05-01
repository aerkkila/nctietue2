/*OPERATION is first defined in wanted way and this file is then included into the code*/

OPERATION(NC_BYTE, hhi, char, pluseq, +=)
OPERATION(NC_UBYTE, hhu, unsigned char, pluseq, +=)
OPERATION(NC_CHAR, c, char, pluseq, +=)
OPERATION(NC_SHORT, hi, short, pluseq, +=)
OPERATION(NC_USHORT, hu, unsigned short, pluseq, +=)
OPERATION(NC_INT, i, int, pluseq, +=)
OPERATION(NC_UINT, u, unsigned, pluseq, +=)
OPERATION(NC_UINT64, llu, long long unsigned, pluseq, +=)
OPERATION(NC_INT64, lli, long long int, pluseq, +=)
OPERATION(NC_FLOAT, .4f, float, pluseq, +=)
OPERATION(NC_DOUBLE, .4lf, double, pluseq, +=)

OPERATION(NC_BYTE, hhi, char, minuseq, -=)
OPERATION(NC_UBYTE, hhu, unsigned char, minuseq, -=)
OPERATION(NC_CHAR, c, char, minuseq, -=)
OPERATION(NC_SHORT, hi, short, minuseq, -=)
OPERATION(NC_USHORT, hu, unsigned short, minuseq, -=)
OPERATION(NC_INT, i, int, minuseq, -=)
OPERATION(NC_UINT, u, unsigned, minuseq, -=)
OPERATION(NC_UINT64, llu, long long unsigned, minuseq, -=)
OPERATION(NC_INT64, lli, long long int, minuseq, -=)
OPERATION(NC_FLOAT, .4f, float, minuseq, -=)
OPERATION(NC_DOUBLE, .4lf, double, minuseq, -=)

OPERATION(NC_BYTE, hhi, char, muleq, *=)
OPERATION(NC_UBYTE, hhu, unsigned char, muleq, *=)
OPERATION(NC_CHAR, c, char, muleq, *=)
OPERATION(NC_SHORT, hi, short, muleq, *=)
OPERATION(NC_USHORT, hu, unsigned short, muleq, *=)
OPERATION(NC_INT, i, int, muleq, *=)
OPERATION(NC_UINT, u, unsigned, muleq, *=)
OPERATION(NC_UINT64, llu, long long unsigned, muleq, *=)
OPERATION(NC_INT64, lli, long long int, muleq, *=)
OPERATION(NC_FLOAT, .4f, float, muleq, *=)
OPERATION(NC_DOUBLE, .4lf, double, muleq, *=)

OPERATION(NC_BYTE, hhi, char, diveq, /=)
OPERATION(NC_UBYTE, hhu, unsigned char, diveq, /=)
OPERATION(NC_CHAR, c, char, diveq, /=)
OPERATION(NC_SHORT, hi, short, diveq, /=)
OPERATION(NC_USHORT, hu, unsigned short, diveq, /=)
OPERATION(NC_INT, i, int, diveq, /=)
OPERATION(NC_UINT, u, unsigned, diveq, /=)
OPERATION(NC_UINT64, llu, long long unsigned, diveq, /=)
OPERATION(NC_INT64, lli, long long int, diveq, /=)
OPERATION(NC_FLOAT, .4f, float, diveq, /=)
OPERATION(NC_DOUBLE, .4lf, double, diveq, /=)

OPERATION(NC_BYTE, hhi, char, modeq, %=)
OPERATION(NC_UBYTE, hhu, unsigned char, modeq, %=)
OPERATION(NC_CHAR, c, char, modeq, %=)
OPERATION(NC_SHORT, hi, short, modeq, %=)
OPERATION(NC_USHORT, hu, unsigned short, modeq, %=)
OPERATION(NC_INT, i, int, modeq, %=)
OPERATION(NC_UINT, u, unsigned, modeq, %=)
OPERATION(NC_UINT64, llu, long long unsigned, modeq, %=)
OPERATION(NC_INT64, lli, long long int, modeq, %=)

OPERATION(NC_BYTE, hhi, char, bitoreq, |=)
OPERATION(NC_UBYTE, hhu, unsigned char, bitoreq, |=)
OPERATION(NC_CHAR, c, char, bitoreq, |=)
OPERATION(NC_SHORT, hi, short, bitoreq, |=)
OPERATION(NC_USHORT, hu, unsigned short, bitoreq, |=)
OPERATION(NC_INT, i, int, bitoreq, |=)
OPERATION(NC_UINT, u, unsigned, bitoreq, |=)
OPERATION(NC_UINT64, llu, long long unsigned, bitoreq, |=)
OPERATION(NC_INT64, lli, long long int, bitoreq, |=)

OPERATION(NC_BYTE, hhi, char, bitandeq, &=)
OPERATION(NC_UBYTE, hhu, unsigned char, bitandeq, &=)
OPERATION(NC_CHAR, c, char, bitandeq, &=)
OPERATION(NC_SHORT, hi, short, bitandeq, &=)
OPERATION(NC_USHORT, hu, unsigned short, bitandeq, &=)
OPERATION(NC_INT, i, int, bitandeq, &=)
OPERATION(NC_UINT, u, unsigned, bitandeq, &=)
OPERATION(NC_UINT64, llu, long long unsigned, bitandeq, &=)
OPERATION(NC_INT64, lli, long long int, bitandeq, &=)

OPERATION(NC_BYTE, hhi, char, bitxoreq, ^=)
OPERATION(NC_UBYTE, hhu, unsigned char, bitxoreq, ^=)
OPERATION(NC_CHAR, c, char, bitxoreq, ^=)
OPERATION(NC_SHORT, hi, short, bitxoreq, ^=)
OPERATION(NC_USHORT, hu, unsigned short, bitxoreq, ^=)
OPERATION(NC_INT, i, int, bitxoreq, ^=)
OPERATION(NC_UINT, u, unsigned, bitxoreq, ^=)
OPERATION(NC_UINT64, llu, long long unsigned, bitxoreq, ^=)
OPERATION(NC_INT64, lli, long long int, bitxoreq, ^=)

OPERATION(NC_BYTE, hhi, char, bitlshifteq, <<=)
OPERATION(NC_UBYTE, hhu, unsigned char, bitlshifteq, <<=)
OPERATION(NC_CHAR, c, char, bitlshifteq, <<=)
OPERATION(NC_SHORT, hi, short, bitlshifteq, <<=)
OPERATION(NC_USHORT, hu, unsigned short, bitlshifteq, <<=)
OPERATION(NC_INT, i, int, bitlshifteq, <<=)
OPERATION(NC_UINT, u, unsigned, bitlshifteq, <<=)
OPERATION(NC_UINT64, llu, long long unsigned, bitlshifteq, <<=)
OPERATION(NC_INT64, lli, long long int, bitlshifteq, <<=)

OPERATION(NC_BYTE, hhi, char, bitrshifteq, >>=)
OPERATION(NC_UBYTE, hhu, unsigned char, bitrshifteq, >>=)
OPERATION(NC_CHAR, c, char, bitrshifteq, >>=)
OPERATION(NC_SHORT, hi, short, bitrshifteq, >>=)
OPERATION(NC_USHORT, hu, unsigned short, bitrshifteq, >>=)
OPERATION(NC_INT, i, int, bitrshifteq, >>=)
OPERATION(NC_UINT, u, unsigned, bitrshifteq, >>=)
OPERATION(NC_UINT64, llu, long long unsigned, bitrshifteq, >>=)
OPERATION(NC_INT64, lli, long long int, bitrshifteq, >>=)