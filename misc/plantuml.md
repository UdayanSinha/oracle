# PlantUML Workflow


## Notes

- Tested on Ubuntu 24.04 LTS.


## References

1. [PlantUML Documentation](https://plantuml.com/en-dark/guide)


## Setup

```console
sudo apt install plantuml
```


## Example PlantUML Sequence Diagram

```uml
@startuml
'This is a comment and will not be rendered

EntityA -> EntityB: SignalAtoBReq
note left
This sends
a signal
to B from A
end note
EntityC -> EntityA: SignalCtoAReq
note right: This sends a signal to A from C

alt SignalReject
EntityA -> EntityC: SignalCtoARej
loop 1000 times
    ...wait for 1 second...
    EntityC -> EntityA: SpamSignal
end
end

== End Of Cross-Communication ==
note over EntityA
Preparing to send
signal to self
end note

EntityA -> EntityA: SignalToSelf
@enduml
```


## PlantUML Rendering

```console
plantuml /path/to/uml-file
```
