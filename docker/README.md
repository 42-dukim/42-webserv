# Docker Dev Container

이 디렉터리는 `clang-format`과 `cppcheck`를 포함한 개발용 컨테이너 구성을 제공합니다. 호스트에서 `sudo` 권한이 없을 때도 동일한 도구를 사용할 수 있습니다.

## 준비

프로젝트 루트에서 다음 명령을 실행합니다.

```bash
cd docker
docker compose build
```

## 도구 사용 예시

컨테이너 안에서 원하는 명령을 실행할 수 있습니다.

- 셸 진입:
  ```bash
  docker compose run --rm dev
  ```
- check-bash 실행
  ```bash
  bash docker/ci-check/ci-check.bash
  ```

## k6 부하 테스트 컨테이너

HTTP 부하 테스트는 `grafana/k6` 이미지를 이용한 별도 서비스로 실행할 수 있습니다.

- ip 주소 찾기
  ```bash
  ip addr show
  ```
  - 해당 명령어를 통해 컨테이너에서 접근 가능한 호스트 IP로 교체하세요
- 기본 GET 부하 테스트:
  ```bash
  docker compose run --rm \
    -e K6_TARGET=http://...:8080/ \
    k6 run docker/k6/k6_basic.js
  ```
- 파일 업로드 혼합 시나리오:
  ```bash
  docker compose run --rm \
    -e K6_TARGET=http://...:8080 \
    -e K6_POST_RATIO=0.7 \
    -e K6_FILE_BYTES=131072 \
    k6 run docker/k6/k6_upload.js
  ```

컨테이너에서 생성된 파일은 호스트의 작업 디렉터리에 바로 반영됩니다.
