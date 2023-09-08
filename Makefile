REPO=docker.au-zone.com/maivin
TAG=testing
BUILD=Release
OUTDIR=$(PWD)/apps

all: help

help:
	@echo make apps
	@echo \ \ \ \ Builds the wayland and headless apps and places them into $(PWD)
	@echo make docker REPO=$(REPO) TAG=$(TAG)
	@echo \ \ \ \ Builds the docker images.  REPO and TAG can be left out.
	@echo make publish
	@echo \ \ \ \ Publishes the docker images.  Will build and uses REPO and TAG

TARGETS := facedetect

apps: $(addsuffix gl,$(TARGETS))
# headless: $(addsuffix gl_headless,$(TARGETS))
# docker: $(addsuffix _docker,$(TARGETS))
# publish: $(addsuffix _publish,$(TARGETS))
# latest: $(addsuffix _latest,$(TARGETS))

$(OUTDIR):
	@mkdir -p $(OUTDIR)

vpkui:
	docker build . --pull --build-arg BUILD_TYPE=$(BUILD) --tag $@:$(TAG)

%gl: # vpkui $(OUTDIR)
	docker build . --pull --build-arg BUILD_TYPE=$(BUILD) --tag $@:$(TAG) $(DOCKARGS)
	docker run --user $(shell id -u):$(shell id -g) --rm -v ${PWD}/apps:/src/copy_mount --platform linux/arm64/v8 --entrypoint cp $@:$(TAG) /src/build/$@ /src/copy_mount/$@ 

# %gl_headless: vpkui $(OUTDIR)
# 	docker build . --pull --build-arg BUILD_TYPE=$(BUILD) --target $@ --tag $@:$(TAG) $(DOCKARGS)
# 	docker run --user $(shell id -u):$(shell id -g) --rm -v $(PWD)/apps:/app --entrypoint /bin/cp --platform linux/arm64/v8 $@:$(TAG) /src/build/samples/$@ /app/

# %_docker: vpkui
# 	docker build . --pull --build-arg BUILD_TYPE=$(BUILD) --target $* \
# 		--tag $(REPO)/$*:$(TAG) $(DOCKARGS) \
# 		--label "maintainer=Au-Zone Technologies" \
# 		--label "org.opencontainers.image.vendor=Au-Zone Technologies" \
# 		--label "org.opencontainers.image.title=DeepView AI Application Zoo" \
# 		--label "org.opencontainers.image.version=$(TAG)" \
# 		--label "org.opencontainers.image.revision=$(GIT_COMMIT)" \
# 		--label "org.opencontainers.image.build=$(BUILD_NUMBER)" \
# 		--label "org.opencontainers.image.url=https://support.deepviewml.com/hc/en-us/articles/6937198744077-VisionPack-AI-Application-Zoo"

%_publish: %_docker
	docker push $(REPO)/$*:$(TAG)

%_latest:
	docker tag $(REPO)/$*:$(TAG) $(REPO)/$*:latest
	docker push $(REPO)/$*:latest

clean:
	$(RM) $(addprefix $(OUTDIR)/,$(addsuffix gl,$(TARGETS)))
	$(RM) $(addprefix $(OUTDIR)/, $(addsuffix gl_headless,$(TARGETS)))
