# Fuzix Security Policy

This is very simple. The code runs on MMUless, protectionless mostly 8bit machines
so there is none. Things like Unix file permissions exist help users avoid
accidents.

Somewhat ridiculous to have to say this but apparently neccessary.

## Things That Are Security Relevant

- Problems in build and standalone tools run on an actually secure system whilst building Fuzix.
- Bugs in network exposed code that are exploitable without any kind of login.

## Things That Are Merely Bugs or Irrelevant

- Stuff that crashes, exploits or otherwise makes apps misbehave
- The fact you can trivially write programs that do anything.

App misbeaviours we care about but as bugs not security holes so please do
report them and post fixes. If you have something which applies to Fuzix as
a bug and to other protected mode systems as a security hole then please
file a bug but feel free to do it after fixing and disclosure on systems
where it actually matters.
