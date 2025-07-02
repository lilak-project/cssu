## downloading docker image

### Windows
  - docker desktop 실행
  - ubuntu 실행 (윈도우 키 누르고 unbuntu)
  - (7Gb 정도의 저장공간 필요)
  - docker pull ejungwoo/cssu_image:latest
 
### Mac
  - docker desktop 실행
  - terminal 실행 (돋보기 누르고 terminal)
  - (7Gb 정도의 저장공간 필요)
  - docker pull ejungwoo/cssu_image:latest

---

## running docker container

### Windows

* 처음 돌릴 때
```
docker run -it \
    -e DISPLAY=$DISPLAY \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    --device /dev/dri \
    --privileged \
    --name cssu_container \
    ejungwoo/cssu_image:latest /bin/bash
```

* 다시 돌릴때
```
docker start cssu_container
docker exec -it cssu_container /bin/bash
```
 
### Mac

* 처음 돌릴 때
```
docker run -it \
    -e DISPLAY=host.docker.internal:0 \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    --device /dev/dri \
    --privileged \
    --name cssu_container \
    ejungwoo/cssu_image:latest /bin/bash
```

* 다시 돌릴때
```
docker start cssu_container
docker exec -it cssu_container /bin/bash
```
 
