# Kusanagi Project

This project was initiated around 2007.
Here is an excerption from the top page at that time.

```
	Kusanagi is a collection of tools intended for trouble shooting
	including both user space and kernel space tools.

	Granted that there are many very useful tools applicable for our
	trouble	shooting activities, but often those tools are inconvenient,
	have restrictions, difficult to understand their outputs or
	extract conclusive results etc.
	From another point of view, often we want to have a collected
	knowledge base to exploit those convenient tools.

	The resources of Kusanagi Project are intended for those kind of needs.
	Tools and documents here cover the light blue region of the left
	picture, and they would help you to analyze system behavior,
	catch evidences	for your guesses or reproduce troubles etc.
```

I thought it could be a good idea to use this project as a placeholder
for various small works for system operation, trouble shooting etc.


At the moment, there are several stil alive utilities.

```
.
|-- linux            # Outdated. You can use ebpf.
|   |-- allie
|   |-- holden
|   |-- rye
|   `-- sigprof
|-- user
|   |-- cpuload      # still alive. It's like 'stress-ng -c'
|   |-- ldas
|   |-- opanalyze
|   |-- pfr          # Still alive. A stupid data recorder for /proc/xxx
|   |-- shmload      # Still alive. It's like 'stress-ng --shm'
|   `-- vgstat       # Still avlie. Gets information from ESXi hypervisor.
`-- web
```
