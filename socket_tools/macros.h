// macros.h
#define EXIT_ON_ERR(fname, skt)({ \
    printf("%s() Failed:%d\n", fname, WSAGetLastError()); \
    return -1; \
})
#define EXIT_AND_CLOSE_SOCK_ON_ERR(fname, skt)({ \
    printf("%s() Failed:%d\n", fname, WSAGetLastError()); \
    closesocket(skt); \
    return -1; \
})
#define TRY_2_P_FUNC(func, p1, p2, err, fname)({ \
    if(func(p1, p2) == err){ \
        EXIT_AND_CLOSE_SOCK_ON_ERR(fname, p1); \
    } \
})
#define TRY_3_P_FUNC(func, p1, p2, p3, err, fname)({ \
    if(func(p1, p2, p3) == err){ \
        EXIT_AND_CLOSE_SOCK_ON_ERR(fname, p1); \
    } \
})
#define TRY_4_P_FUNC(func, p1, p2, p3, p4, err, fname)({ \
    if(func(p1, p2, p3, p4) == err){ \
        EXIT_AND_CLOSE_SOCK_ON_ERR(fname, p1); \
    } \
})
#define TRY_5_P_FUNC(func, p1, p2, p3, p4, p5, err, fname)({ \
    if(func(p1, p2, p3, p4, p5) == err){ \
        EXIT_AND_CLOSE_SOCK_ON_ERR(fname, p1); \
    } \
})
#define TRY_6_P_FUNC(func, p1, p2, p3, p4, p5, p6, err, fname)({ \
    if(func(p1, p2, p3, p4, p5, p6) == err){ \
        EXIT_AND_CLOSE_SOCK_ON_ERR(fname, p1); \
    } \
})
#define TEST_CREATED_SOCK(csck, err, fname)({ \
    if(csck == err){ \
        EXIT_ON_ERR(fname, csck); \
    } \
})
#define TEST_RECEIVED_SOCK(rsck, err, fname)({ \
    if(rsck == err){ \
        EXIT_AND_CLOSE_SOCK_ON_ERR(fname, rsck); \
    } \
})
