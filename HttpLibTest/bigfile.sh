


curl -v -u admin:123456 \
    -X PUT \
    -T test/design_top_slogan.png \
    "http://127.0.0.1:8080/bigfile/maven/test/com/example/test/nexus-3.84.0-03-mac-aarch_64.tar.gz"


curl -v -u admin:123456 \
    -X PUT \
    -T test/nexus-3.84.0-03-mac-aarch_64.tar.gz \
    "http://127.0.0.1:8080/bigfile/maven/test/com/example/test/nexus-3.84.0-03-mac-aarch_64.tar.gz"

curl -v -u admin:123456 \
    -X PUT \
    -T test/nexus-3.84.0-03-mac-aarch_64.tar.gz \
    "http://127.0.0.1:8080/bigmemoryfile/maven/test/com/example/test/nexus-3.84.0-03-mac-aarch_64.tar.gz"

