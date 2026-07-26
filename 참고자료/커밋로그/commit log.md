# 커밋 로그

---

## [chore] filter_data.proto / person.proto 제거 및 관련 소스 정리

### 삭제 파일
- `TestMFC/protobuf/filter_data.proto`
- `TestMFC/protobuf/filter_data.pb.h`
- `TestMFC/protobuf/filter_data.pb.cc`
- `TestMFC/protobuf/person.proto`
- `TestMFC/protobuf/person.pb.h`
- `TestMFC/protobuf/person.pb.cc`
- `TestMFC/TestMFCDlg.cpp` (SendDlg로 이름 변경되기 전 미사용 잔재 파일 — vcxproj 빌드 목록에 없었음)

### 수정 파일
- `TestMFC/CopyDataDefs.h`
- `TestMFC/ReceiverDlg.h`
- `TestMFC/ReceiverDlg.cpp`
- `TestMFC/SendDlg.h`
- `TestMFC/SendDlg.cpp`
- `TestMFC/TestMFC.rc`
- `TestMFC/resource.h`
- `TestMFC/TestMFC.vcxproj`
- `TestMFC/TestMFC.vcxproj.filters`

### 변경 배경
`filter_data.proto`, `person.proto` 2개 프로토 정의를 프로젝트에서 완전히 제거하기로 결정.
두 메시지(`tutorial::Person`, `my_msg::FilterConfig`)는 오직 "Send 1" 데모 버튼 한 곳에서만
송수신되고 있어, 버튼/핸들러/리소스까지 함께 제거해야 잔재 없이 정리됨.

### 제거 항목
| 항목 | 설명 |
|---|---|
| `COPYDATA_PERSON_PROTO = 9001` | CopyDataDefs.h 상수 삭제 |
| `COPYDATA_FILTER_DATA = 9002` | CopyDataDefs.h 상수 삭제 |
| `IDC_BUTTON_SEND1 (1000)` | resource.h 삭제 |
| `OnBnClickedButtonSend1` | SendDlg / ReceiverDlg 핸들러 전체 삭제 (Person + FilterConfig 생성·직렬화·전송 로직 포함) |
| RC 버튼 1개 | `"Send 1"` PUSHBUTTON (IDD_TESTMFC_DIALOG) |
| `#include "protobuf/filter_data.pb.h"` / `"protobuf/person.pb.h"` | ReceiverDlg.cpp, SendDlg.cpp |
| `OnCopyData` 분기 2개 | `COPYDATA_PERSON_PROTO`, `COPYDATA_FILTER_DATA` 역직렬화·표시 로직 (ReceiverDlg.cpp, SendDlg.cpp) |
| `CSendDlg::SaveProtobufData` / `LoadProtobufData` 주석 처리 예제 | SendDlg.h 선언부, SendDlg.cpp 파일 하단 구현부 (미사용 `my_msg::FilterConfig` 참조) |
| vcxproj / vcxproj.filters 항목 | `ClInclude`, `ClCompile`, `None`(.proto) 각 2건씩 제거 |

### 주의
- `IDC_BUTTON_SEND1`과 `IDC_BUTTON_SEND`가 동일 값(1000)으로 정의돼 있었음 — `IDC_BUTTON_SEND1`만 삭제, 다른 곳에서 쓰이는 `IDC_BUTTON_SEND`는 유지.
- `TestMFC.rc`는 UTF-16LE(BOM) 인코딩이라 PowerShell(`Get-Content -Encoding Unicode` / `[System.IO.File]::WriteAllLines`)로 인코딩 보존하며 편집.

---

## [feat] BindValue.valen 제거 → binaryVal(bytes) + binaryLen(uint32) 반영

### 수정 파일
- `TestMFC/protobuf/max_orange_protocol.proto`
- `TestMFC/SendDlg.cpp`
- `TestMFC/ReceiverDlg.cpp`

### 변경 내용
- proto: `BindValue` 필드 6 `uint32 valen` 삭제 → `bytes binaryVal = 6` 추가
- proto: `BindValue` 필드 7 `uint32 binaryLen = 7` 추가
- `OnCopyData` / 수신 로그 3곳 (BindValue / BindData / WVMultiBindData) 수정:
  - 포맷 문자열: `valen: %u` → `binaryVal 크기: %u bytes  binaryLen: %u`
  - 포맷 문자열(per-item): `valen=%u` → `binaryVal크기=%u  binaryLen=%u`
  - args: `oMsg.valen()` / `oBv.valen()` → `(uint32_t)oMsg.binaryval().size(), oMsg.binarylen()` 등
- 송신 함수 예제 데이터: `set_valen(N)` → `set_binarylen(N)` 전량 교체
  - SendDlg.cpp: BindValue(1) + BindData(2) + WVMultiBindData(12) 총 15곳
  - ReceiverDlg.cpp: BindValue(1) + BindData(2) + WVMultiBindData(12) 총 15곳

---

## [feat] WVMultiBindData — tmp_bind_column 제거 및 bind_data 예제 3행으로 확충

### 수정 파일
- `TestMFC/protobuf/max_orange_protocol.proto`
- `TestMFC/protobuf/max_orange_protocol.pb.cc` (protoc 재생성)
- `TestMFC/protobuf/max_orange_protocol.pb.h`  (protoc 재생성)
- `TestMFC/SendDlg.cpp`
- `TestMFC/ReceiverDlg.cpp`

### 변경 내용
- `WVMultiBindData.tmp_bind_column` (field 9) proto에서 제거 — 의미 없는 필드
- OnCopyData WVMultiBindData 분기에서 tmp_bind_column 표시 블록 제거
- `OnBnClickedBtnWvMultiBindData` 예제 데이터를 bind_data 3행으로 확충
  - SendDlg   : EMP 테이블 배치 INSERT (EMPNO/ENAME/SAL/DEPTNO, 3행)
  - ReceiverDlg: CUSTOMER 테이블 배치 INSERT (CUSTNO/CUSTNAME/GRADE/AMT, 3행)

---

## [feat] proto 변경 반영 — WVMultiSQLData3U/WVSQLData 제거, WVMultiSQLData4 추가

### 수정 파일
- `TestMFC/CopyDataDefs.h`
- `TestMFC/resource.h`
- `TestMFC/SendDlg.h`
- `TestMFC/SendDlg.cpp`
- `TestMFC/ReceiverDlg.h`
- `TestMFC/ReceiverDlg.cpp`
- `TestMFC/TestMFC.rc`

### 변경 배경
`max_orange_protocol.proto` 에서 `WVMultiSQLData3U`, `WVSQLData` 메시지가 삭제되고
`QIDQueryLimit` + `WVMultiSQLData4` 메시지가 신규 추가됨.
이에 따라 로직(송수신 버튼 핸들러, OnCopyData 분기, 리소스 버튼)을 동기화.

### 제거 항목
| 항목 | 설명 |
|---|---|
| `COPYDATA_WV_MULTISQL3U = 9022` | CopyDataDefs.h 상수 삭제 |
| `COPYDATA_WV_SQLDATA = 9023` | CopyDataDefs.h 상수 삭제 |
| `IDC_BTN_WV_MULTISQLDATA3U (1017)` | resource.h — ID 재사용 |
| `IDC_BTN_WV_SQLDATA (1018)` | resource.h 삭제 |
| `OnBnClickedBtnWvMultiSqlData3U` | SendDlg / ReceiverDlg 핸들러 삭제 |
| `OnBnClickedBtnWvSqlData` | SendDlg / ReceiverDlg 핸들러 삭제 |
| RC 버튼 2개 | `WVMultiSQLData3U`, `WVSQLData` |

### 추가 항목
| 항목 | 설명 |
|---|---|
| `COPYDATA_WV_MULTISQLDATA4 = 9029` | CopyDataDefs.h 신규 상수 |
| `IDC_BTN_WV_MULTISQLDATA4 (1017)` | resource.h — 기존 1017 재사용 |
| `OnBnClickedBtnWvMultiSqlData4` | SendDlg / ReceiverDlg 핸들러 추가 |
| OnCopyData 분기 추가 | `WVMultiSQLData4` 역직렬화 + 표시 |
| RC 버튼 1개 | `WVMultiSQLData4(1)`, y=279 |

### WVMultiSQLData4 필드 구성
```proto
message QIDQueryLimit { string sql=1; string qid=2; uint32 limit=3; }
message WVMultiSQLData4 {
    WVSessionData session_data = 1;
    repeated QIDQueryLimit qid_limit = 2;
    uint32 handle = 3;
    uint32 exec_cnt = 4;
}
```

### RC 버튼 y좌표 조정 (2개 제거 → 이후 버튼 17씩 상승)
| 버튼 | 이전 y | 이후 y |
|---|---|---|
| WVMultiSQLData4(1) | (신규) | 279 |
| WVSaveDbConnInfo(1) | 313 | 296 |
| WVPathData | 330 | 313 |
| BindValue | 347 | 330 |
| BindData(1) | 364 | 347 |
| WVMultiBindData(2) | 381 | 364 |

---

## [feat] 버튼 캡션에 메시지 중첩 깊이 표기 추가

### 수정 파일
- `TestMFC/TestMFC.rc`

### 변경 배경
proto 메시지 객체를 중첩으로 포함하는 버튼 캡션에 최대 중첩 깊이(depth)를
`MessageName(N)` 형식으로 표기. 깊이가 없는 메시지(원시 타입만 포함)는 변경 없음.

### 중첩 깊이 계산 기준
- depth N = 해당 메시지에서 sub-message 체인의 최대 깊이
- 원시 타입(int32, string 등)만 가지는 메시지 = 깊이 없음 → 표기 생략

### 변경 전 → 후

| 변경 전 | 변경 후 | 이유 |
|---|---|---|
| WVProtocol | WVProtocol(1) | ctrl + oneof{WVSupportDBMSData, WVSessionData} |
| WVSessionDataSet | WVSessionDataSet(1) | repeated WVSessionData |
| WVMultiSQLData | WVMultiSQLData(1) | WVSessionData |
| WVSQLArrayData | WVSQLArrayData(1) | WVSessionData |
| WVOutputData | WVOutputData(2) | WVSQLArrayData → WVSessionData |
| WVFileData | WVFileData(3) | WVOutputData → WVSQLArrayData → WVSessionData |
| WVSingleSQLData3 | WVSingleSQLData3(1) | WVSessionData |
| WVMultiSQLData3 | WVMultiSQLData3(1) | WVSessionData |
| WVMultiLimitData3 | WVMultiLimitData3(1) | WVSessionData |
| WVMultiSQLData3U | WVMultiSQLData3U(1) | WVSessionData + repeated WVSQLData |
| WVSaveDbConnInfo | WVSaveDbConnInfo(1) | WVSaveDbServerPasswordOption |
| BindData | BindData(1) | repeated BindValue |
| WVMultiBindData | WVMultiBindData(2) | repeated BindData → repeated BindValue |

### 변경 없는 버튼 (원시 타입만 보유)
WVProtocolCtrl, WVConnectData, WVSessionData, WVSupportDBMSData,
WVSaveDbSvrPwdOpt, WVSQLData, WVPathData, BindValue

---

## [feat] OnCopyData 전체 메시지 타입 수신 표시 상세 강화

### 수정 파일
- `TestMFC/SendDlg.cpp`
- `TestMFC/ReceiverDlg.cpp`

### 변경 배경
`WVMultiSQLData` 에 적용한 상세 출력 패턴을 나머지 10개 메시지 타입에 동일하게 적용.
개수만 표시하던 repeated 필드, 서브 메시지를 모두 루프 열거하도록 강화.

### 수정 대상 블록 목록

| 메시지 타입 | 강화 내용 |
|---|---|
| WVSessionDataSet | session_data[i] 10개 필드 전부 출력 (기존: ip/port만) |
| WVSQLArrayData | session_data 10개 + sql 목록 인덱스별 열거 (기존: ip+개수만) |
| WVOutputData | sql_array_data.session_data 10개 + sql 목록 열거 (기존: type/copy_size만) |
| WVFileData | file_path + output_data.type/copy_size + session_data 10개 + sql 목록 (기존: file_path만) |
| WVSingleSQLData3 | session_data 10개 + sql/qid/limit (기존: sql/qid/limit만) |
| WVMultiSQLData3 | session_data 10개 + sql 목록 + qid 목록 인덱스별 열거 (기존: 개수만) |
| WVMultiLimitData3 | session_data 10개 + qid 목록 + limit 목록 인덱스별 열거 (기존: 개수만) |
| WVMultiSQLData3U | session_data 10개 + sqls[i].sql/qid/limit 열거 (기존: 개수만) |
| BindData | bind_value[i] 6개 필드(type/qid/idx/name/val/valen) 열거 (기존: 개수만) |
| WVMultiBindData | bind_data[i]→bind_value[j] + tmp_bind_column[i] 열거 (기존: 개수만) |

---

## [feat] WVMultiSQLData OnCopyData — repeated string sql 항목별 상세 출력

### 수정 파일
- `TestMFC/SendDlg.cpp`
- `TestMFC/ReceiverDlg.cpp`

### 변경 배경
`COPYDATA_WV_MULTISQLDATA` 수신 시 `sql_size()` 개수만 표시하고
`repeated string sql` 의 각 항목 내용을 보여주지 않아 디버깅이 어려웠음.
`session_data` 필드도 ip/port 수준이 아닌 10개 전체 필드를 출력하도록 강화.

### 수신 출력 변경 (OnCopyData — COPYDATA_WV_MULTISQLDATA 블록)

#### 변경 전
```
== WVMultiSQLData 수신 ==
session ip: 192.168.1.100
sql 수: 2
크기: 42 bytes
```

#### 변경 후
```
== WVMultiSQLData 수신 ==
[session_data]
  dbms_type : 1
  ip        : 192.168.1.100
  port      : 1521
  database  : ORCL
  protocol  : TCP
  db_info_1 : charset=UTF8
  db_info_2 : timeout=30
  db_info_3 : pool=5
  user      : admin
  password  : secret
[sql 목록] (총 2 개)
  [0] SELECT * FROM EMP
  [1] SELECT * FROM DEPT
크기: 62 bytes
```

### 핵심 코드 패턴
```cpp
const WVSessionData& oSession = oMsg.session_data();
CString strDisplay;
strDisplay.Format( /* session_data 10개 필드 + sql 총 개수 */ ... );

for (int i = 0; i < oMsg.sql_size(); ++i)
{
    strDisplay.AppendFormat(_T("  [%d] %S\r\n"),
        i, oMsg.sql(i).c_str());
}
strDisplay.AppendFormat(_T("크기: %d bytes"), pCopyDataStruct->cbData);
```

### 동일 적용 파일
- `SendDlg.cpp::OnCopyData` (행 367~382 교체)
- `ReceiverDlg.cpp::OnCopyData` (행 319~333 교체)

---

## [feat] max_orange_protocol 21개 메시지 타입별 버튼 추가 (UI + 송수신 코드)

### 수정 파일
- `TestMFC/resource.h`
- `TestMFC/CopyDataDefs.h`
- `TestMFC/TestMFC.rc`
- `TestMFC/SendDlg.h`
- `TestMFC/SendDlg.cpp`
- `TestMFC/ReceiverDlg.h`
- `TestMFC/ReceiverDlg.cpp`

### 변경 배경
`max_orange_protocol.proto` 에 정의된 21개 메시지 타입 각각에 대해
독립적인 버튼과 WM_COPYDATA 송수신 예제 코드를 추가.

### UI 변경 (TestMFC.rc)
- 다이얼로그 크기: `579×200` → `580×432`
- 좌측 버튼 컬럼 (x=7, w=105): Send1, Send2 + 21개 프로토콜 버튼
- 편집 컨트롤 (IDC_EDIT_RECV): 우측 (x=117, w=456, h=395)으로 재배치

### 버튼 목록 (IDC_BTN_WV_* : 1003~1023)

| # | 버튼 표시명 | IDC 식별자 | COPYDATA ID |
|---|------------|-----------|------------|
| 1 | WVProtocolCtrl | IDC_BTN_WV_PROTOCOLCTRL | 9010 |
| 2 | WVProtocol | IDC_BTN_WV_PROTOCOL | 9003 (기존) |
| 3 | WVConnectData | IDC_BTN_WV_CONNECTDATA | 9004 (기존) |
| 4 | WVSessionData | IDC_BTN_WV_SESSIONDATA | 9011 |
| 5 | WVSessionDataSet | IDC_BTN_WV_SESSIONDATASET | 9012 |
| 6 | WVSupportDBMSData | IDC_BTN_WV_SUPPORTDBMSDATA | 9013 |
| 7 | WVMultiSQLData | IDC_BTN_WV_MULTISQLDATA | 9014 |
| 8 | WVSQLArrayData | IDC_BTN_WV_SQLARRAYDATA | 9015 |
| 9 | WVOutputData | IDC_BTN_WV_OUTPUTDATA | 9016 |
| 10 | WVFileData | IDC_BTN_WV_FILEDATA | 9017 |
| 11 | WVSaveDbSvrPwdOpt | IDC_BTN_WV_SAVEDBSVRPWDOPT | 9018 |
| 12 | WVSingleSQLData3 | IDC_BTN_WV_SINGLESQLDATA3 | 9019 |
| 13 | WVMultiSQLData3 | IDC_BTN_WV_MULTISQLDATA3 | 9020 |
| 14 | WVMultiLimitData3 | IDC_BTN_WV_MULTILIMITDATA3 | 9021 |
| 15 | WVMultiSQLData3U | IDC_BTN_WV_MULTISQLDATA3U | 9022 |
| 16 | WVSQLData | IDC_BTN_WV_SQLDATA | 9023 |
| 17 | WVSaveDbConnInfo | IDC_BTN_WV_SAVEDBCONNINFO | 9024 |
| 18 | WVPathData | IDC_BTN_WV_PATHDATA | 9025 |
| 19 | BindValue | IDC_BTN_WV_BINDVALUE | 9026 |
| 20 | BindData | IDC_BTN_WV_BINDDATA | 9027 |
| 21 | WVMultiBindData | IDC_BTN_WV_MULTIBINDDATA | 9028 |

### 코드 구조
- 각 버튼 핸들러: 샘플 데이터로 해당 메시지 직렬화 → WM_COPYDATA 전송 → AfxMessageBox 결과 표시
- `OnCopyData`: 19개 신규 분기 추가 (기존 WVProtocol/WVConnectData 재사용)
- `SendDlg` → `ReceiverApp` 전송 / `ReceiverDlg` → `SendApp` 전송

---

## [feat] WVSessionData 전체 10필드 + WVConnectData 예제 코드 추가

### 수정 파일
- `TestMFC/CopyDataDefs.h`
- `TestMFC/SendDlg.cpp`
- `TestMFC/ReceiverDlg.cpp`

### 변경 배경
Send2 버튼의 `WVSessionData` 예제가 10개 필드 중 일부(db_info_1/2/3, protocol, password)를
누락하고 있었음. `WVConnectData` (접속 직렬화 전용 마커 메시지) 예제가 없었음.
양쪽 다이얼로그에 누락 필드 및 신규 메시지 타입을 보완.

### CopyDataDefs.h
```cpp
static const DWORD COPYDATA_WV_CONNECT = 9004;  // WVConnectData — 접속 직렬화 전용
```

### Send2 버튼 동작 (SendDlg / ReceiverDlg 공통)

| 단계 | 전송 메시지 | COPYDATA ID |
|------|------------|-------------|
| 2단계 | WVProtocol (WVSessionData / WVSupportDBMSData) | 9003 |
| 3단계 | WVConnectData (마커, 필드 없음 → 0 bytes) | 9004 |

### WVSessionData 전체 10개 필드 (SendDlg Send2)

| 필드 | 설정값 |
|------|--------|
| dbms_type | 1 |
| ip | "192.168.1.100" |
| port | 1521 |
| database | "ORCL" |
| protocol | "TCP" |
| db_info_1 | "charset=UTF8" |
| db_info_2 | "timeout=30" |
| db_info_3 | "pool=5" |
| user | "admin" |
| password | "secret" |

### WVConnectData 처리 특이사항
- proto3 빈 메시지 → `SerializeToString` 결과 0 bytes
- 송신: `cbData=0`, `lpData=nullptr` 으로 WM_COPYDATA 전송
- 수신: `cbData > 0` 일 때만 `ParseFromArray` 호출, `cbData=0` 이면 빈 객체로 처리

### OnCopyData 수신 표시 확장 (양쪽 공통)
WVSessionData 분기에 프로토콜/DB정보1·2·3/패스워드 5개 필드 추가 표시.

---

---

## [refactor] ReceiverDlg — 전송 버튼을 Send1 / Send2 로 분리

### 수정 파일
- `TestMFC/ReceiverDlg.cpp`

### 변경 배경
SendDlg 와 동일한 구조로 맞추기 위해 ReceiverDlg 의 `OnBnClickedButtonSend1()` 에
혼재되어 있던 WVProtocol 블록을 분리. 기존 빈 스텁이던 `OnBnClickedButtonSend2()` 를
max_orange_protocol.proto 전용으로 구현.

### 버튼 ↔ 함수 ↔ 프로토콜 매핑

| 버튼 ID | RC 캡션 | 함수 | 담당 proto |
|---------|---------|------|------------|
| `IDC_BUTTON_SEND1` (1000) | "Send 1" | `OnBnClickedButtonSend1` | `person.proto` + `filter_data.proto` |
| `IDC_BUTTON_SEND2` (1002) | "Send 2" | `OnBnClickedButtonSend2` | `max_orange_protocol.proto` (WVProtocol) |

### ReceiverDlg.cpp 변경 내용

#### 메시지맵
```cpp
// 변경 전
ON_BN_CLICKED(IDC_BUTTON_SEND, &CReceiverDlg::OnBnClickedButtonSend1)

// 변경 후
ON_BN_CLICKED(IDC_BUTTON_SEND1, &CReceiverDlg::OnBnClickedButtonSend1)
```

#### OnBnClickedButtonSend1() — person + filter 전용 (4단계)
- WVProtocol 블록(구 4~5단계) 제거
- 결과 출력: Person + FilterConfig 만 표시

#### OnBnClickedButtonSend2() — WVProtocol 전용 (3단계) [신규 구현]
1. SendApp 윈도우 검색
2. `WVProtocol(WVSupportDBMSData, RET_SUCCESS)` 직렬화 → `COPYDATA_WV_PROTOCOL(9003)` 전송
3. 결과 출력 (AfxMessageBox)

### 동작 불변 항목
- `ReceiverDlg.h` 선언 변경 없음 (이미 Send1/Send2 선언 존재)
- `OnCopyData()` 수신 처리 변경 없음

---

---

## [refactor] SendDlg — 전송 버튼을 Send1 / Send2 로 분리

### 수정 파일
- `TestMFC/SendDlg.h`
- `TestMFC/SendDlg.cpp`

### 변경 배경
기존 `OnBnClickedButtonSend1()` 한 함수에 person/filter/WVProtocol 세 가지 프로토콜이
모두 혼재되어 있었음. 프로토콜 계열별로 버튼을 분리해 독립 테스트가 가능하도록 재구성.

### 버튼 ↔ 함수 ↔ 프로토콜 매핑

| 버튼 ID | RC 캡션 | 함수 | 담당 proto |
|---------|---------|------|------------|
| `IDC_BUTTON_SEND1` (1000) | "Send 1" | `OnBnClickedButtonSend1` | `person.proto` + `filter_data.proto` |
| `IDC_BUTTON_SEND2` (1002) | "Send 2" | `OnBnClickedButtonSend2` | `max_orange_protocol.proto` (WVProtocol) |

### SendDlg.h
```cpp
// 변경 전
afx_msg void OnBnClickedButtonSend();

// 변경 후
afx_msg void OnBnClickedButtonSend1();   // Send1 : person.proto + filter.proto
afx_msg void OnBnClickedButtonSend2();   // Send2 : max_orange_protocol.proto
```

### SendDlg.cpp — 메시지맵
```cpp
// 변경 전
ON_BN_CLICKED(IDC_BUTTON_SEND, &CSendDlg::OnBnClickedButtonSend1)

// 변경 후
ON_BN_CLICKED(IDC_BUTTON_SEND1, &CSendDlg::OnBnClickedButtonSend1)
ON_BN_CLICKED(IDC_BUTTON_SEND2, &CSendDlg::OnBnClickedButtonSend2)
```

### OnBnClickedButtonSend1() — person + filter 전용 (4단계)
1. ReceiverApp 윈도우 검색
2. `tutorial::Person` 직렬화 → `COPYDATA_PERSON_PROTO(9001)` 전송
3. `my_msg::FilterConfig` 직렬화 → `COPYDATA_FILTER_DATA(9002)` 전송
4. Person / FilterConfig 결과 출력 (AfxMessageBox)

### OnBnClickedButtonSend2() — WVProtocol 전용 (3단계) [신규]
1. ReceiverApp 윈도우 검색
2. `WVProtocol(WVSessionData, MNG_ADD)` 직렬화 → `COPYDATA_WV_PROTOCOL(9003)` 전송
3. WVProtocol 결과 출력 (AfxMessageBox)

### 동작 불변 항목
- `ReceiverDlg.cpp` 수신 처리(`OnCopyData`) 변경 없음
- RC 버튼 배치 변경 없음 (`IDC_BUTTON_SEND1`, `IDC_BUTTON_SEND2` 이미 존재)

---

---

## [feat] max_orange_protocol — WVProtocol 송수신 로직 추가

### 수정 파일
- `TestMFC/CopyDataDefs.h`
- `TestMFC/SendDlg.cpp`
- `TestMFC/ReceiverDlg.cpp`

### 변경 배경
`max_orange_protocol.proto`에 정의된 `WVProtocol` 메시지를 기존 `Person` / `FilterConfig`
전송 구조에 추가. `WM_COPYDATA` 기반의 프로세스 간 통신(IPC) 패턴을 유지하면서
새 프로토콜 타입 식별자(`COPYDATA_WV_PROTOCOL = 9003`)를 추가함.

### 흐름 요약

| 방향 | 메시지 | oneof | ctrl.sub_cmd |
|------|--------|-------|--------------|
| SendDlg → ReceiverDlg | WVProtocol | WVSessionData (DB 접속 정보) | MNG_ADD (0) |
| ReceiverDlg → SendDlg | WVProtocol | WVSupportDBMSData (DBMS 타입) | RET_SUCCESS (1) |

### CopyDataDefs.h
```cpp
static const DWORD COPYDATA_WV_PROTOCOL = 9003;  // max_orange_protocol (WVProtocol)
```

### SendDlg.cpp

#### OnBnClickedButtonSend() — 4단계 추가 (기존 3단계 뒤에 삽입)
```cpp
// 4단계: WVProtocol(WVSessionData) 생성 → 직렬화 → ReceiverApp 전송
WVProtocol oWvProtocol;
WVProtocolCtrl* pCtrl = oWvProtocol.mutable_ctrl();
pCtrl->set_sub_cmd(WVProtocolCtrl::MNG_ADD);
pCtrl->set_err_msg("");

WVSessionData* pSessionData = oWvProtocol.mutable_wvsessiondata();
pSessionData->set_dbms_type(1);
pSessionData->set_ip("192.168.1.100");
pSessionData->set_port(1521);
pSessionData->set_database("ORCL");
pSessionData->set_protocol("TCP");
pSessionData->set_user("admin");
pSessionData->set_password("secret");
// SerializeToString → COPYDATASTRUCT → SendMessage(WM_COPYDATA)
```

#### OnCopyData() — COPYDATA_WV_PROTOCOL 수신 처리 추가
- `ParseFromArray` 로 `WVProtocol` 역직렬화
- `msg_case()` 분기: `has_wvsessiondata()` / `has_wvsupportdbmsdata()` 로 표시

### ReceiverDlg.cpp

#### OnCopyData() — COPYDATA_WV_PROTOCOL 수신 처리 추가
- `ParseFromArray` 로 `WVProtocol` 역직렬화
- `has_wvsessiondata()` 분기 → IP / 포트 / DB / 사용자 표시

#### OnBnClickedButtonSend() — 4단계 추가 (기존 3단계 뒤에 삽입)
```cpp
// 4단계: WVProtocol(WVSupportDBMSData) 생성 → 직렬화 → SendApp 전송
WVProtocol oWvProtocol;
WVProtocolCtrl* pCtrl = oWvProtocol.mutable_ctrl();
pCtrl->set_sub_cmd(WVProtocolCtrl::RET_SUCCESS);
pCtrl->set_err_msg("");

WVSupportDBMSData* pSupportData = oWvProtocol.mutable_wvsupportdbmsdata();
pSupportData->set_dbms_type(1);
// SerializeToString → COPYDATASTRUCT → SendMessage(WM_COPYDATA)
```

### 사용 enum 값 (WVProtocolCtrl 내부)

| 상수 | 값 | 용도 |
|------|----|------|
| `WVProtocolCtrl::MNG_ADD` | 0 | 세션 추가 요청(Send→Receiver) |
| `WVProtocolCtrl::RET_SUCCESS` | 1 | 처리 성공 응답(Receiver→Send) |

---

---

## [refactor] Ade 3 버튼(IDC_BUTTON99)을 IDD_ETC → IDD_V78(V8열 하단)으로 이동

### 수정 파일
- `OrangeExecutor/OExecutor.rc`
- `OrangeExecutor/V7tab.cpp`
- `OrangeExecutor/ETCtab.cpp`
- `OrangeExecutor/OExecutorDlg.cpp`

### 변경 배경
IDD_ETC 3열에 있던 Ade 3 STD-U 버튼(64BIT 전용)을 IDD_V78의 Orange 8.0(V8) 열 하단에 배치.
Ade 3은 64BIT 단일 버튼이므로 V8 열(x=735) 64BIT 그룹 아래(y=310~375)에 적층 배치.

### RC 레이아웃 변경 (OExecutor.rc)

#### IDD_V78 추가 (Orange 8.0 64BIT 하단, x=735)
| 항목 | 좌표 (x,y,w,h) | 비고 |
|------|----------------|------|
| GROUPBOX "Ade 3" | 735,310,85,65 | 외부 그룹박스 |
| GROUPBOX "64BIT" | 735,322,85,48 | 내부 그룹박스 |
| STD-U (IDC_BUTTON99) | 740,337,32,15 | 64BIT 전용 |
| IDC_EDIT99 | 775,337,40,15 | 버전 표시 |

Ade 3 그룹박스 하단(y=375) < 다이얼로그 높이(385) ✓

#### IDD_ETC 제거
- `// -- 3열: Ade 3` 주석 제거
- `GROUPBOX "Ade 3"` (255,10,105,115) 제거
- `GROUPBOX "64BIT"` (260,80,95,40) 제거
- `IDC_BUTTON99` (265,95,32,15) 제거
- `IDC_EDIT99` (300,95,45,15) 제거

### V7tab.cpp
```cpp
BEGIN_MESSAGE_MAP(CV7Tab, CDialogEx)
    ON_COMMAND_RANGE(IDC_BUTTON1,   IDC_BUTTON52,  OnToolExecution)
    ON_COMMAND_RANGE(IDC_BUTTON99,  IDC_BUTTON99,  OnToolExecution) // Ade 3 추가
    ON_COMMAND_RANGE(IDC_BUTTON102, IDC_BUTTON103, OnToolExecution)
END_MESSAGE_MAP()
```

### ETCtab.cpp
IDC_BUTTON99(=1098)가 IDC_BUTTON93~101 범위 중간에 위치하여 두 범위로 분리:
```cpp
// 변경 전
ON_COMMAND_RANGE(IDC_BUTTON93, IDC_BUTTON101, OnToolExecution)

// 변경 후
ON_COMMAND_RANGE(IDC_BUTTON93, IDC_BUTTON98,  OnToolExecution)
ON_COMMAND_RANGE(IDC_BUTTON100, IDC_BUTTON101, OnToolExecution)
```

### OExecutorDlg.cpp — EditUpdate()

#### V7EDIT 케이스 — Ade 3 블록 추가
```cpp
// Ade 3 STD-U (m_aOrangeInfo[98] → dlg1 IDC_BUTTON99)
{
    int i = IDC_BUTTON99 - IDC_BUTTON1;  // = 98
    OrangeInfo oInfo = m_aOrangeInfo.GetAt(i);
    if (oInfo.bExists == TRUE)
    {
        dlg1->GetDlgItem(IDC_BUTTON1 + i)->EnableWindow(TRUE);
        dlg1->SetDlgItemText(IDC_EDIT1 + i, oInfo.sVersion);
    }
    else
    {
        dlg1->GetDlgItem(IDC_BUTTON1 + i)->EnableWindow(FALSE);
        dlg1->SetDlgItemText(IDC_EDIT1 + i, _T(""));
    }
}
```

#### VETCEDIT 케이스 — i=98 스킵
```cpp
for (int i = 92; i <= nRange; ++i) // 92~100 (IDC_BUTTON99 제외)
{
    if (i == IDC_BUTTON99 - IDC_BUTTON1) // Ade3은 dlg1(V78)으로 이동
    {
        continue;
    }
    ...
}
```

### 데이터 인덱스 매핑
| 인덱스 | 경로 | 버튼 ID | 변경 전 위치 | 변경 후 위치 |
|--------|------|---------|------------|------------|
| 98 | Orange Ade 3.0\OrangeU.exe (64BIT STD-U) | IDC_BUTTON99 (1098) | dlg3 (IDD_ETC) | dlg1 (IDD_V78) |

`OnGetExecutionPath`의 `m_aOrangeInfo.GetAt(nId - IDC_BUTTON1)` 공식은 변경 없이 그대로 동작.

---

## [refactor] IDD_V78 컬럼 표시 순서 재배치 (ORA-TIBERO-ALTIBASE-DB2-SQL-ASE-IQ-PETA-V8)

### 수정 파일
- `OrangeExecutor/OExecutor.rc`

### 변경 배경
IDD_V78 다이얼로그의 DB 종류별 컬럼 배치를 사용 빈도 및 관련성 기준으로 재배치.
버튼 ID, 데이터 매핑 공식(`m_aOrangeInfo.GetAt(nId - IDC_BUTTON1)`), .cpp 파일은 일절 변경 없음.

### RC 레이아웃 변경 (OExecutor.rc — IDD_V78)

#### 컬럼 X 좌표 변경

| 새 열 | DB 종류 | 이전 x (기준점) | 이후 x (기준점) | 델타 |
|-------|---------|----------------|----------------|------|
| 1열 | ORA | 15 | 15 | 0 (유지) |
| 2열 | TIBERO (← 5열) | 375 | 105 | -270 |
| 3열 | ALTIBASE (← 4열) | 285 | 195 | -90 |
| 4열 | DB2 (← 2열) | 105 | 285 | +180 |
| 5열 | SQL SERVER (← 3열) | 195 | 375 | +180 |
| 6열 | SYBASE ASE (← 7열) | 555 | 465 | -90 |
| 7열 | SYBASE IQ (← 6열) | 465 | 555 | +90 |
| 8열 | PETA SQL | 645 | 645 | 0 (유지) |
| 9열 | Orange 8.0 (V8) | 735 | 735 | 0 (유지) |

내부 컨트롤 규칙: 열 기준점 x에 대해 버튼 x=기준+5, 에디트 x=기준+40.

#### 레이아웃 가이드 주석 업데이트 (IDD_V78 상단)

```
// 변경 전
//-- ORA(15) | DB2(105) | SQL(195) | ALTI(285) | TIBERO(375) | IQ(465) | ASE(555) | PETA(645)

// 변경 후
//-- ORA(15) | TIBERO(105) | ALTI(195) | DB2(285) | SQL(375) | ASE(465) | IQ(555) | PETA(645) | V8(735)
```

#### 열 헤더 주석 번호 업데이트
- `// -- 2열: DB2` → `// -- 4열: DB2`
- `// -- 3열: SQL SERVER` → `// -- 5열: SQL SERVER`
- `// -- 4열: ALTIBASE` → `// -- 3열: ALTIBASE`
- `// -- 5열: TIBERO` → `// -- 2열: TIBERO`
- `// -- 6열: SYBASE IQ` → `// -- 7열: SYBASE IQ`
- `// -- 7열: SYBASE ASE` → `// -- 6열: SYBASE ASE`

### 동작 불변 항목 확인

| 항목 | 상태 |
|------|------|
| 버튼 ID (IDC_BUTTON1~52, IDC_BUTTON102~103) | 변경 없음 |
| 데이터 매핑 공식 (`GetAt(nId - IDC_BUTTON1)`) | 변경 없음 |
| V7tab.cpp, ETCtab.cpp, OExecutorDlg.cpp | 변경 없음 |
| 최종 화면 표시 순서 | ORA-TIBERO-ALTIBASE-DB2-SQL SERVER-SYBASE ASE-SYBASE IQ-PETA SQL-V8 |

---

## [refactor] Orange 8.0 버튼(IDC_BUTTON102, IDC_BUTTON103)을 IDD_ETC → IDD_V78으로 이동

### 수정 파일
- `OrangeExecutor/OExecutor.rc`
- `OrangeExecutor/V7tab.cpp`
- `OrangeExecutor/ETCtab.cpp`
- `OrangeExecutor/OExecutorDlg.cpp`

### 변경 배경
IDD_ETC 다이얼로그의 4열(Orange 8.0) 영역에 있던 STD-U / DBA-U 버튼 및 에디트 박스를
IDD_V78(V7 탭) 다이얼로그의 9열로 이동. Orange 8.0은 64BIT 전용이므로
IDD_V78의 64BIT 영역(y=175~305) 위치에 배치함.

### RC 레이아웃 변경 (OExecutor.rc)

#### IDD_V78
| 항목 | 변경 |
|------|------|
| 다이얼로그 너비 | 780 → 870 (9열 공간 확보, 열 간격 90 DLU) |
| 9열 헤더 GROUPBOX | Orange 8.0, x=735, y=5, w=85, h=25 |
| 9열 64BIT GROUPBOX | 64BIT, x=735, y=175, w=85, h=130 |
| STD-U (IDC_BUTTON102) | x=740, y=206, w=32, h=15 (64BIT STD-U 위치) |
| IDC_EDIT102 | x=775, y=206, w=40, h=15 |
| DBA-U (IDC_BUTTON103) | x=740, y=242, w=32, h=15 (64BIT DBA-U 위치) |
| IDC_EDIT103 | x=775, y=242, w=40, h=15 |

#### IDD_ETC
- 4열 "Orange 8.0" 섹션 전체 제거
  - GROUPBOX "Orange 8.0" (375,10,105,115)
  - GROUPBOX "64BIT" (380,30,95,85)
  - IDC_BUTTON102, IDC_EDIT102, IDC_BUTTON103, IDC_EDIT103

### V7tab.cpp
- `ON_COMMAND_RANGE(IDC_BUTTON1, IDC_BUTTON52, ...)` 유지
- `ON_COMMAND_RANGE(IDC_BUTTON102, IDC_BUTTON103, OnToolExecution)` 추가

### ETCtab.cpp
- `ON_COMMAND_RANGE(IDC_BUTTON93, IDC_BUTTON103, ...)` →
  `ON_COMMAND_RANGE(IDC_BUTTON93, IDC_BUTTON101, ...)`

### OExecutorDlg.cpp — EditUpdate()

#### V7EDIT 케이스
```cpp
// 기존: IDC_BUTTON1~52 (i=0~51) dlg1 업데이트
// 추가: IDC_BUTTON102~103 (i=101~102) dlg1 업데이트
for (int i = IDC_BUTTON102 - IDC_BUTTON1; i <= IDC_BUTTON103 - IDC_BUTTON1; ++i)
{
    OrangeInfo oInfo = m_aOrangeInfo.GetAt(i);  // m_arrPath[101~102]
    if (oInfo.bExists == TRUE)
    {
        dlg1->GetDlgItem(IDC_BUTTON1 + i)->EnableWindow(TRUE);
        dlg1->SetDlgItemText(IDC_EDIT1 + i, oInfo.sVersion);
    }
    else
    {
        dlg1->GetDlgItem(IDC_BUTTON1 + i)->EnableWindow(FALSE);
        dlg1->SetDlgItemText(IDC_EDIT1 + i, _T(""));
    }
}
```

#### VETCEDIT 케이스
```cpp
// 변경 전
nRange = (IDC_EDIT103 - IDC_EDIT93) + 92;  // i: 92~102 (dlg3)
// 변경 후
nRange = (IDC_EDIT101 - IDC_EDIT93) + 92;  // i: 92~100 (dlg3), 101~102는 dlg1로 이동
```

### 데이터 인덱스 매핑 (m_arrPath)
| 인덱스 | 경로 | 버튼 ID | 표시 위치 |
|--------|------|---------|-----------|
| 101 | Orange 8.0 STD-U | IDC_BUTTON102 (1101) | dlg1 (IDD_V78) |
| 102 | Orange 8.0 DBA-U | IDC_BUTTON103 (1102) | dlg1 (IDD_V78) |

`OnGetExecutionPath`의 `m_aOrangeInfo.GetAt(nId - IDC_BUTTON1)` 공식은 변경 없이 그대로 동작.

---

## [feat] FilterListDlg 트리 컬럼 크기 축소 및 다이얼로그 레이아웃 재배치

### 수정 파일
- `MFCApplication8/MFCApplication8/FilterListDlg.cpp`
- `MFCApplication8/MFCApplication8/MFCApplication8.rc`

### 변경 배경
트리 그리드 컬럼 너비를 축소하면서 RC 다이얼로그 전체 폭 및 컨트롤 위치가
이전 컬럼 크기를 기준으로 설계되어 있었으므로 함께 조정.

### 컬럼 변경 (FilterListDlg.cpp)

| 컬럼 | 이전 (px) | 이후 (px) |
|------|-----------|-----------|
| Col 0: Tree Name | 160 | 120 |
| Col 1: Enable    | 45  | 35  |
| Col 2: Filter Name | 150 | 65 |
| **합계** | **355** | **220** |

### RC 레이아웃 변경 (MFCApplication8.rc — IDD_FILTER_LIST_DLG)

비례 계산: 감소량 135px / 355px * 411 DLU ≈ **156 DLU 축소**

| 항목 | 이전 (DLU) | 이후 (DLU) |
|------|-----------|-----------|
| 다이얼로그 너비 | 656 | 500 |
| Filter List 그룹박스 너비 | 422 | 266 |
| IDC_TREE_FILTER_LIST 너비 | 411 | 255 |
| 우측 패널 모든 컨트롤 x | 각 x 값 | x - 156 |
| DESIGNINFO RIGHTMARGIN | 651 | 495 |

#### 우측 패널 주요 컨트롤 x 좌표 변경

| 컨트롤 | 이전 x | 이후 x |
|--------|--------|--------|
| IDC_CHECK_ENABLE | 438 | 282 |
| IDC_CHECK_ALL_SCHEMA | 581 | 425 |
| Include/Exclude 그룹박스 | 436 | 280 |
| 리스트박스 (Include/Exclude) | 441 | 285 |
| Add/Remove/Clear 버튼 | 601 | 445 |
| IDC_STATIC_MASK_HINT | 441 | 285 |
| Saved filter 그룹박스 | 436 | 280 |
| IDC_COMBO_FILTER_NAME | 468 | 312 |
| IDC_BUTTON_SAVE | 581 | 425 |
| IDC_BUTTON_SAVED_REMOVE | 610 | 454 |
| IDOK | 484 | 328 |
| IDCANCEL | 551 | 395 |

### OnInitDialog() 변경 없음
`m_bShowFilterList = FALSE` 모드의 레이아웃 재배치 로직은
실행 시 실제 픽셀 위치를 동적으로 계산하므로 하드코딩된 크기 값이 없음.
RC 변경만으로 자동 적용됨.

---

## [fix] 표시 전용 노드 nullptr 제거 — m_nTreeLevel 기반 표시 전용 판정으로 변경

### 수정 파일
- `MFCApplication8/MFCApplication8/FilterListDlg.cpp`

### 변경 배경
이전 구현에서 L1 스키마 노드(`PopulateTree`)와 L3 개별 객체 노드(`BuildSubTree`)에
`FilterKeyword* = nullptr` 을 ItemData 로 저장하였음.

이 때문에 해당 노드 선택 시 `m_pCurrentKW = nullptr` 이 되어,
`OnPopupRemoveDependentFilters` / `OnPopupAllClearFilter` 에서 null 체크로 조기 반환하거나
크래시가 발생하는 문제가 있었음.

표시 전용 여부는 포인터 null 이 아닌 `m_nTreeLevel` 필드로 판단하도록 변경.

### 레벨별 m_nTreeLevel 및 처리 방식

| 레벨 | 대상 예시 | m_nTreeLevel | 처리 |
|------|-----------|--------------|------|
| L0 | 연결 노드 | 0 | 편집 가능 |
| L1 | 스키마 (SCOTT 등) | 1 | 표시 전용 |
| L2 | 카테고리 (Tables 등) | 2 | 편집 가능 |
| L3 | 개별 객체 (emp 등) | 3 | 표시 전용 |
| L4 | 서브카테고리 (Columns 등) | 4 | 편집 가능 |

### 수정 내용

#### AddTreeNode() — 표시 전용 판정 변경
```cpp
// 변경 전
if (pKW) { /* Y/N */ }
else     { /* "-"  */ }  // pKW==nullptr 이면 "-"

// 변경 후
bool bDisplayOnly = (!pKW || pKW->m_nTreeLevel == 1 || pKW->m_nTreeLevel == 3);
if (!bDisplayOnly) { /* Y/N */ }
else               { /* "-" */ }
```

#### PopulateTree() — L1 스키마 노드에 실제 FilterKeyword* 전달
```cpp
// 변경 전
CXTPGridRecord* pSchema = AddTreeNode(pConn->GetChilds(), sSchema, nullptr);

// 변경 후
kwConn.m_nTreeLevel = 0;  // 연결 노드 레벨 설정
// ...
FilterKeyword& kwSchema = kwConn.m_mapChildFilter[sSchema];  // 없으면 기본값 생성
kwSchema.m_nTreeLevel = 1;
CXTPGridRecord* pSchema = AddTreeNode(pConn->GetChilds(), sSchema, &kwSchema);
```

#### BuildSubTree() — 항상 실제 FilterKeyword* 전달, m_nTreeLevel 설정
```cpp
// 변경 전
FilterKeyword* pKW = (nDepth == 1) ? nullptr : &entry.second;
CXTPGridRecord* pRecord = AddTreeNode(pRecords, entry.first, pKW);

// 변경 후
entry.second.m_nTreeLevel = (nDepth == 0) ? 2 : (nDepth == 1) ? 3 : 4;
CXTPGridRecord* pRecord = AddTreeNode(pRecords, entry.first, &entry.second);
```

#### LoadNodeToPanel() — m_nTreeLevel 기준으로 표시 전용 판정
```cpp
// 변경 전
if (!pKW) { SetRightPanelEnabled(FALSE); return; }

// 변경 후
bool bDisplayOnly = (!pKW || pKW->m_nTreeLevel == 1 || pKW->m_nTreeLevel == 3);
if (bDisplayOnly) { SetRightPanelEnabled(FALSE); return; }
// ※ m_pCurrentKW = pKW 는 항상 유효 → 컨텍스트 메뉴 함수 정상 동작
```

#### SavePanelToNode() — 표시 전용 레벨 저장 스킵
```cpp
if (!m_pCurrentKW) { return; }

// 추가
if (m_pCurrentKW->m_nTreeLevel == 1 || m_pCurrentKW->m_nTreeLevel == 3)
{
    return;  // 표시 전용 노드: 패널 내용 저장 불가
}
```

### 동작 결과

| 선택 노드 | m_pCurrentKW | 패널 | OnPopupRemoveDependentFilters |
|-----------|--------------|------|-------------------------------|
| L0 연결 | 유효 (level=0) | 활성 | 정상 동작 |
| L1 스키마 | 유효 (level=1) | **비활성** | **정상 동작** ← 수정됨 |
| L2 카테고리 | 유효 (level=2) | 활성 | 정상 동작 |
| L3 개별 객체 | 유효 (level=3) | **비활성** | **정상 동작** ← 수정됨 |
| L4 서브카테고리 | 유효 (level=4) | 활성 | 정상 동작 |

---

## [feat] 표시 전용 노드 선택 시 우측 패널 전체 비활성화 (SetRightPanelEnabled)

### 수정 파일
- `MFCApplication8/MFCApplication8/FilterListDlg.h`
- `MFCApplication8/MFCApplication8/FilterListDlg.cpp`

### 변경 배경
`"-"` 로 표기되는 표시 전용 노드(L1 스키마, L3 개별 객체)를 트리에서 선택하면
우측 패널의 모든 컨트롤이 활성 상태로 유지되어 사용자가 조작 가능한 것처럼 보임.
실제로는 해당 레벨에서 필터 설정이 불가능하므로 컨트롤 전체를 비활성화해야 함.

### 비활성 대상 컨트롤 (표시 전용 노드 선택 시)
- `IDC_CHECK_ENABLE` / `IDC_CHECK_ALL_SCHEMA`
- `IDC_STATIC_GROUP_INCLUDE`, `IDC_LIST_INCLUDE`, `IDC_BUTTON_INC_ADD/REMOVE/CLEAR`
- `IDC_STATIC_GROUP_EXCLUDE`, `IDC_LIST_EXCLUDE`, `IDC_BUTTON_EXC_ADD/REMOVE/CLEAR`
- `IDC_STATIC_MASK_HINT`
- `IDC_COMBO_FILTER_NAME`, `IDC_BUTTON_SAVE`, `IDC_BUTTON_SAVED_REMOVE`

### 수정 내용

#### FilterListDlg.h — SetRightPanelEnabled 선언 추가
```cpp
void SetRightPanelEnabled(BOOL bEnabled);  // 우측 패널 전체 컨트롤 활성/비활성
```

#### FilterListDlg.cpp — SetRightPanelEnabled 구현
```cpp
void CFilterListDlg::SetRightPanelEnabled(BOOL bEnabled)
{
    m_chkEnable.EnableWindow(bEnabled);
    m_chkAllSchema.EnableWindow(bEnabled);

    GetDlgItem(IDC_STATIC_GROUP_INCLUDE)->EnableWindow(bEnabled);
    m_lstInclude.EnableWindow(bEnabled);
    m_buttonIncAdd.EnableWindow(bEnabled);
    m_buttonIncRemove.EnableWindow(bEnabled);
    m_buttonIncClear.EnableWindow(bEnabled);

    GetDlgItem(IDC_STATIC_GROUP_EXCLUDE)->EnableWindow(bEnabled);
    m_lstExclude.EnableWindow(bEnabled);
    m_buttonExcAdd.EnableWindow(bEnabled);
    m_buttonExcRemove.EnableWindow(bEnabled);
    m_buttonExcClear.EnableWindow(bEnabled);

    GetDlgItem(IDC_STATIC_MASK_HINT)->EnableWindow(bEnabled);

    m_cboName.EnableWindow(bEnabled);
    GetDlgItem(IDC_BUTTON_SAVE)->EnableWindow(bEnabled);
    GetDlgItem(IDC_BUTTON_SAVED_REMOVE)->EnableWindow(bEnabled);
}
```

#### FilterListDlg.cpp — LoadNodeToPanel 수정
```cpp
void CFilterListDlg::LoadNodeToPanel(FilterKeyword* pKW)
{
    ...
    if (!pKW)
    {
        // 표시 전용 노드: 패널 초기화 후 전체 비활성화
        m_chkEnable.SetCheck(BST_UNCHECKED);
        m_cboName.SetWindowText(_T(""));
        SetRightPanelEnabled(FALSE);   // ← 신규
        return;
    }

    // 편집 가능 노드: 전체 활성화 후 내용 설정 (이전에 비활성화됐을 수 있으므로 먼저 재활성화)
    SetRightPanelEnabled(TRUE);        // ← 신규
    m_chkEnable.SetCheck(...);
    ...
    UpdateEnableState();  // Enable 체크박스 기준으로 Include/Exclude 세부 조정
}
```

#### FilterListDlg.cpp — OnGridSelChanged 수정
```cpp
// 변경 전: nullptr 이면 LoadNodeToPanel 호출 스킵
if (pKW)
    LoadNodeToPanel(pKW);

// 변경 후: nullptr 이어도 항상 호출 → LoadNodeToPanel 내부에서 비활성화 처리
LoadNodeToPanel(pKW);
```

### 동작 흐름 요약

| 선택 노드 | pKW | SetRightPanelEnabled | UpdateEnableState |
|---|---|---|---|
| L0 연결 | 유효 | TRUE | Enable 체크박스 기준 조정 |
| L1 스키마 | nullptr | FALSE (전체 비활성) | 호출 안 함 |
| L2 카테고리 | 유효 | TRUE | Enable 체크박스 기준 조정 |
| L3 개별 객체 | nullptr | FALSE (전체 비활성) | 호출 안 함 |
| L4 서브카테고리 | 유효 | TRUE | Enable 체크박스 기준 조정 |

---

## [fix] L3 개별 객체 레벨 표시 전용 처리 (BuildSubTree nDepth 파라미터 추가)

### 수정 파일
- `MFCApplication8/MFCApplication8/FilterListDlg.h`
- `MFCApplication8/MFCApplication8/FilterListDlg.cpp`

### 변경 배경
FilterListDlg 트리에서 L3 개별 객체 레벨(bbbbbb, blobtest, emp 등 개별 테이블/뷰 이름)이
Enable 열에 "N"으로 표시되고 있었으나, 해당 레벨은 실질적으로 필터 설정이 불가능한 레벨임.
L1 스키마 노드와 동일하게 표시 전용("-")으로 처리되어야 함.

### 레벨별 필터 설정 가능 여부

| 레벨 | 대상 예시 | 처리 방식 | Enable 열 |
|------|-----------|-----------|-----------|
| L0 | 연결 노드 (1 - SCOTT@...) | 필터 설정 가능 | Y/N |
| L1 | 스키마 (SCOTT, HR 등) | 표시 전용 | - |
| L2 | 카테고리 (Tables, Views 등) | 필터 설정 가능 | Y/N |
| **L3** | **개별 객체 (bbbbbb, emp 등)** | **표시 전용 ← 이번 수정** | **-** |
| L4 | 서브카테고리 (Columns, Indexes 등) | 필터 설정 가능 | Y/N |

### 수정 내용

#### FilterListDlg.h
`BuildSubTree` 시그니처에 `nDepth` 파라미터(기본값 0) 추가:
```cpp
// 변경 전
void BuildSubTree(CXTPGridRecords* pRecords, FilterKeyword& kwParent);

// 변경 후
void BuildSubTree(CXTPGridRecords* pRecords, FilterKeyword& kwParent, int nDepth = 0);
```

#### FilterListDlg.cpp — BuildSubTree()
```cpp
void CFilterListDlg::BuildSubTree(CXTPGridRecords* pRecords, FilterKeyword& kwParent, int nDepth /*= 0*/)
{
    for (auto& entry : kwParent.m_mapChildFilter)
    {
        // nDepth == 1: L3 개별 객체 레벨 → pKW = nullptr → Enable/FilterName 열에 "-" 표시
        FilterKeyword* pKW = (nDepth == 1) ? nullptr : &entry.second;
        CXTPGridRecord* pRecord = AddTreeNode(pRecords, entry.first, pKW);

        if (!entry.second.m_mapChildFilter.empty())
        {
            BuildSubTree(pRecord->GetChilds(), entry.second, nDepth + 1);
            pRecord->SetExpanded(TRUE);
        }
    }
}
```

#### FilterListDlg.cpp — OnGridSelChanged() 주석 수정
```cpp
// 변경 전
if (pKW)  // nullptr 이면 L1 스키마 노드(표시 전용) → 패널 갱신 스킵

// 변경 후
if (pKW)  // nullptr 이면 표시 전용 노드(L1 스키마, L3 개별 객체) → 패널 갱신 스킵
```

---

## [fix] FilterListDlg 재귀 호출 오류 수정 및 FilterDlgOpen 공통 헬퍼 추가

### 수정 파일
- `MFCApplication8/MFCApplication8/FilterListDlg.cpp`
- `MFCApplication8/MFCApplication8/MFCApplication8Dlg.cpp`
- `MFCApplication8/MFCApplication8/MFCApplication8Dlg.h`
- `MFCApplication8/MFCApplication8/MFCApplication8.rc`
- `MFCApplication8/MFCApplication8/resource.h`

---

### 버그 수정 내용 (FilterListDlg.cpp)

#### 1. [핵심] PopulateTree() — BuildSubTree 재귀 호출 오류

**문제:**
```cpp
// 잘못된 코드
BuildSubTree(pSchema->GetChilds(), kwConn);
```
`kwConn`(연결 노드 전체)을 넘기면, 각 스키마 노드 아래에
`kwConn.m_mapChildFilter` 의 **모든 스키마들**이 중복·재귀적으로 부착됨.

예) `alter_test_user` 스키마 노드 아래에
`alter_test_user`, `autocompletetest`, `scott` … 등 형제 스키마가 모두 달라붙는 현상.

**수정:**
```cpp
// 해당 스키마의 자식 필터만 하위 트리에 구성
auto itSch = kwConn.m_mapChildFilter.find(sSchema);
if (itSch != kwConn.m_mapChildFilter.end())
{
    BuildSubTree(pSchema->GetChilds(), itSch->second);
}
```

#### 2. PopulateTree() — vecSchemas 구성 방식 변경

**문제:**
`kwConn.m_mapChildFilter` 키만 사용 → 초기 빈 맵이면 스키마 노드가 표시되지 않음.

**수정:**
`m_vecSchemaNames` 우선 사용. 비어있을 때만 `kwConn.m_mapChildFilter` 키 폴백.
```cpp
if (!m_vecSchemaNames.empty())
{
    vecSchemas = m_vecSchemaNames;
}
else
{
    for (const auto& it : kwConn.m_mapChildFilter)
    {
        vecSchemas.push_back(it.first);
    }
}
```

#### 3. 생성자 — m_pCurrentKW 미초기화 (UB 방지)

**수정:**
```cpp
CFilterListDlg::CFilterListDlg(CWnd* pParent)
    : CDialogEx(IDD_FILTER_LIST_DLG, pParent)
    , m_pCurrentRecord(nullptr)
    , m_pCurrentKW(nullptr)     // ← 추가
    , m_sConnName(_T(""))
    , m_bShowFilterList(TRUE)
```

---

### 기능 추가 내용 (MFCApplication8Dlg.cpp / .h)

#### 4. FilterDlgOpen() 공통 헬퍼 도입

Button3(트리 표시)과 Button4(트리 미표시) 두 진입점의 중복 코드를
`FilterDlgOpen(BOOL bShowTree)` 헬퍼로 통합.

**MFCApplication8Dlg.h:**
```cpp
afx_msg void OnBnClickedButton3(); // 트리 표시
afx_msg void OnBnClickedButton4(); // 트리 미표시
void FilterDlgOpen(BOOL bShowTree);
```

**MFCApplication8Dlg.cpp:**
```cpp
void CMFCApplication8Dlg::FilterDlgOpen(BOOL bShowTree)
{
    const CString sConn = _T("1 - SCOTT@vm11g - odbc");
    CFilterListDlg dlg(this);
    dlg.m_sConnName       = sConn;
    dlg.m_mapMainFilter   = m_mapMainFilter;  // 복사 먼저
    dlg.m_bShowFilterList = bShowTree;
    dlg.m_vecSchemaNames.push_back(_T("ANONYMOUS"));
    dlg.m_vecSchemaNames.push_back(_T("SCOTT"));
    dlg.m_vecSchemaNames.push_back(_T("HR"));

    // 복사 후 연결 노드 포인터 취득 (복사본에서 취득해야 DoModal 중 유효)
    auto itConn = dlg.m_mapMainFilter.find(sConn);
    if (itConn != dlg.m_mapMainFilter.end())
    {
        dlg.m_pCurrentKW = &itConn->second;
    }

    if (dlg.DoModal() == IDOK)
    {
        m_mapMainFilter = dlg.m_mapMainFilter;
        BuildTree(m_mapMainFilter);
    }
}

void CMFCApplication8Dlg::OnBnClickedButton3() { FilterDlgOpen(TRUE);  }
void CMFCApplication8Dlg::OnBnClickedButton4() { FilterDlgOpen(FALSE); }
```

#### 5. IDC_BUTTON4 리소스 추가

- `resource.h` : `IDC_BUTTON4` ID 정의 추가
- `MFCApplication8.rc` : 다이얼로그에 BUTTON4 컨트롤 추가

---

### 부모 다이얼로그 호출부 예시 (FilterListDlg 사용법)

```cpp
// ── [예시 1] 트리 표시 모드 (전체 필터 편집) ──
void CParentDlg::OnBnClickedBtnFilterList()
{
    const CString sConn = _T("2 - root@vm_8.0.27");
    CFilterListDlg dlg(this);

    dlg.m_sConnName     = sConn;
    dlg.m_mapMainFilter = m_mapMainFilter;  // 복사 먼저
    dlg.m_vecSchemaNames.push_back(_T("alter_test_user"));
    dlg.m_vecSchemaNames.push_back(_T("scott"));
    dlg.m_vecSchemaNames.push_back(_T("hr"));
    dlg.m_bShowAllSchemaApply = TRUE;
    dlg.m_sCheckCaption       = _T("All Schema Apply");
    dlg.m_sObject             = _T("Columns");

    if (dlg.DoModal() == IDOK)
    {
        m_mapMainFilter   = dlg.m_mapMainFilter;
        m_bAllSchemaApply = dlg.m_bAllSchemaApply;
        RebuildTree();
    }
}

// ── [예시 2] 트리 미표시 모드 (단일 노드 간편 편집) ──
void CParentDlg::OnBnClickedBtnQuickFilter()
{
    const CString sConn = _T("2 - root@vm_8.0.27");
    CFilterListDlg dlg(this);

    dlg.m_sConnName       = sConn;
    dlg.m_mapMainFilter   = m_mapMainFilter;  // 복사 먼저
    dlg.m_bShowFilterList = FALSE;
    dlg.m_sObject         = _T("Tables");

    // 복사 완료 후 연결 노드 포인터 취득
    auto itConn = dlg.m_mapMainFilter.find(sConn);
    if (itConn != dlg.m_mapMainFilter.end())
    {
        dlg.m_pCurrentKW = &itConn->second;
    }

    if (dlg.DoModal() == IDOK)
    {
        m_mapMainFilter = dlg.m_mapMainFilter;
        RebuildTree();
    }
}
```

---

## [fix] vcpkg 종속 제거 - protobuf/abseil 링커 설정 로컬화

### 수정 파일
- `TestMFC/TestMFC.vcxproj`
- `lib64/protobuf/Release/` (신규 — vcpkg x64-windows Release lib 파일 29개 추가)

### 변경 배경
vcpkg 의존성을 없애고 protobuf 관련 include / lib / dll을 프로젝트 내부에 심는 작업.
include 는 이미 정리되어 있었으나, lib/dll 이동 후 아래 두 원인으로 빌드 오류 발생:

| 오류 유형 | 원인 |
|---|---|
| `__imp_?combine@MixingHashState...` (LNK2019) | `abseil_dll.lib` 미링크 |
| `?Flush@LogMessage...` (LNK2019) | `ABSL_CONSUMING_DLL` 미정의 → 헤더가 dllimport 없이 선언되어 import lib 심볼(`__imp_xxx`)과 불일치 |

abseil을 DLL로 빌드한 vcpkg 패키지는 `abseil_dll.dll` 하나에 대부분의 심볼을 번들.
소비자 쪽에서 `ABSL_CONSUMING_DLL`을 정의해야 abseil 헤더가 `__declspec(dllimport)` 를 붙여
import lib 항목과 매칭됨.

### 수정 내용 (TestMFC.vcxproj)

#### Debug|x64
| 항목 | 변경 |
|---|---|
| `PreprocessorDefinitions` | `ABSL_CONSUMING_DLL` 추가 |
| `AdditionalDependencies` | `abseil_dll.lib;utf8_range.lib;utf8_validity.lib` 추가 |

#### Release|x64
| 항목 | 변경 |
|---|---|
| `PreprocessorDefinitions` | `ABSL_CONSUMING_DLL` 추가 |
| `AdditionalLibraryDirectories` | `C:\Users\USER\Desktop\Github_clone\vcpkg download\...` (타 PC 하드코딩 절대경로) → `../lib64/protobuf/Release` 교체 |
| `AdditionalDependencies` | `abseil_dll.lib;utf8_range.lib;utf8_validity.lib` 추가 |

### 파일 복사
`vcpkg\installed\x64-windows\lib\` → `lib64\protobuf\Release\` (abseil_dll.lib 포함 29개)
