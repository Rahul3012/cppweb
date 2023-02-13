FROM bbox:latest
WORKDIR /usr/src/cppweb/cppbox/hello_crow
COPY . .
WORKDIR /usr/src/cppweb/cppbox/hello_crow/build
RUN cmake ..
RUN make
CMD ["./hello_crow"]
