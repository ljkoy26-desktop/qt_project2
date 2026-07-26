

# 목표



1. 아래 flag들은 모두 제거 되었어. pb.cc ,pb.h 는 재생성 해놨으니까
사라진 message 들에 대한 afxbutton 이랑 로직을 제거해줘 




message	WVMultiSQLData {
	WVSessionData session_data	=	1;
	repeated string sql			=	2;
}

message WVSQLArrayData {
	WVSessionData session_data	=	1;
	repeated string sql			=	2;

}

message WVSingleSQLData3 {
	WVSessionData session_Data	=	1;
	string sql				=	2;
	string qid				=	3;
	int32 limit				=	4;
}

message WVMultiSQLData3 {
	WVSessionData session_data	=	1;
	repeated string sql		=	2;
	repeated string qid			=	3;
}




message WVMultiLimitData3 {
	WVSessionData session_data	=	1;
	repeated string qid			=	2;
	repeated int32	limit		=	3;
}


message WVMultiSQLData4 {	
	WVSessionData session_data	=	1;	
    repeated QIDQueryLimit qid_limit = 2;
    uint32 handle = 3;
    uint32 exec_cnt = 4;
}

2. 그리고 아래 플래그가 추가되었어. WVMultiSQLData4 에서 
 살짝 변경이 되었고 관련 로직을 추가해줘 

message WVMultiSQLData5 {	
	WVSessionData session_data	=	1;	
    repeated QIDQueryLimit qid_limit = 2;
    uint32 handle = 3;
    uint32 exec_cnt = 4;
	bool load_tool_falg = 5;
}

# 수정 대상



# 요구사항
3. 코드 스타일: 
   - Visual Studio 2019 환경 , C++ 11 표준 사용중
   - Allman 스타일(중괄호 다음 줄), 헝가리언 표기법 준수.
   - 한 줄 제어문도 줄바꿈/중괄호 필수.
   - 인코딩: UTF-8 with BOM.   
4. 기존 로직 보존: 기존 프로젝트의 다른 코드는 건드리지 마세요(재탐색 금지).
5. 응답은 항상 한글로 해주세요.
6. commit 은 하지 않고 소스 경로내에 참고자료/커밋로그 내에 기록해주세요.

# 출력 형식
1. 수정된 코드 블록
2. 적용 후 커밋 로그 (실제 커밋은 하지 마세요)
3. 부모 다이얼로그에서 데이터를 전달하는 호출부 예시 코드