# 上传文件到 Nexus 仓库，通过 -v 查看详细响应
curl -v -u admin:123456 \
  -X PUT \
  -T target/PomTest-1.0-SNAPSHOT.jar \
  "http://localhost:8081/repository/maven-snapshots/com/example/demo/1.0-SNAPSHOT/demo-1.0-SNAPSHOT.jar"


  curl -v -u admin:123456 \
    -X PUT \
    -T target/PomTest-1.0-SNAPSHOT.jar \
    "http://localhost:8081/repository/maven-releases/com/example/demo/1.0-RELEASE/demo-1.0-RELEASE.jar"





curl -v -u admin:123456 \
    -X PUT \
    -T target/PomTest-1.0-SNAPSHOT.jar \
    "http://localhost:8081/repository/maven-public/com/example/demo/1.0-RELEASE/demo-1.0-RELEASE.jar"


curl -v -u admin:123456 \
    http://localhost:8081/repository/maven-public/com/example/demo/1.0-RELEASE/demo-1.0-RELEASE.jar


curl -v -u admin:123456 \
    -H "If-None-Match: \"80dd00aebf9126519ccb1b7d819372c10444009b\"" \
    http://localhost:8081/repository/maven-public/com/example/demo/1.0-RELEASE/demo-1.0-RELEASE.jar


curl -v -u admin:123456 \
    -H "If-None-Match: \"80dd00aebf9126519ccb1b7d819372c10444009b\"" \
    -H "If-Modified-Since: Thu, 11 Sep 2025 12:54:31 GMT" \
    http://localhost:8081/repository/maven-public/com/example/demo/1.0-RELEASE/demo-1.0-RELEASE.jar

curl -v -u admin:123456 \
    -H "If-Modified-Since: Thu, 11 Sep 2025 12:54:31 GMT" \
    http://localhost:8081/repository/maven-public/com/example/demo/1.0-RELEASE/demo-1.0-RELEASE.jar