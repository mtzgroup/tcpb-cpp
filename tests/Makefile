CXX=g++
CXXFLAGS=-std=c++11
LIBS=-lprotobuf -ltcpb

.PHONY: all
all: available_test energy_grad_force_test

available_test: available/available_test.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIBS)
	@mv $@ available/

energy_grad_force_test: energy_grad_force/energy_grad_force_test.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIBS)
	@mv $@ energy_grad_force/

.PHONY: clean
clean:
	@rm available/available_test
	@rm energy_grad_force/energy_grad_force_test
	@rm client.log
