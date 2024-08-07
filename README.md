## 기본 개념
Data는 서버-클라 통신이 기본이다.(클라끼리는 통신X)
클라가 보내는 데이터는 항상 서버가 수신한다.
서버는 이를 다른 클라이언트에게 전달한다.

## 리플리케이션
액터 리플리케이션 -> 로컬 클라(내컴) -> 서버에게 요청, 동일한 트랜스폼의 사본을 만들것을 요청한다. -> 서버는 이 사본들을 다른 프록시(다른 사람 컴)에게 시뮬레이트(흉내) 한다.
변수 리플리케이션 -> UPROPERTY(Replicates)가 마킹된 변수는 서버에서만 값을 업데이트 한다.(단방향. 반대 불가) -> 값이 변경되면 이 값을 클라들에게 다운로드 시키려고 시도한다.(동기화) -> 예) 대미지 처리는 서버만 함 -> 깎인 체력을 모든 클라이언트에게 동기화 시도 -> 해당 클라액터 값 업뎃 -> 값의 변경 여부를 RepNotify로 하이재킹할 수 있다.(이벤트 기반)

## RPC
서버콜, 클라콜, 넷멀티캐스트콜이 있다.
호출된 당사자의 로컬에서 실행X
요청되면 인터럽트 없이 바로 다음줄을 실행함(애니메이션, 타이머처럼)

## 종합
꼭 그런건 아니지만
상태 변경 요청 -> 서버에서 실행하는 것이 원칙
단지 코스매틱 -> 클라에서 실행
라운드트립 타임 때문에 서버값을 수신받을 때 까지 기다리지 않고, 클라에게 예상값을 미리 써버리기도 함