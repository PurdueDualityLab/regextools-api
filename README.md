
# regextools.io - backend services

This repository contains much of the code used for the Regex Reuse project as a whole.
It provides an implementation of the regex search engine along with projects for clustering
regexes, measuring DFA coverage, measuring search engine performance, and some services for
creating a search engine API.

This readme will give an overview of the directory as well as some information on how to build.

## Some Useful Terms
Here are some useful terms that are used in general throughout this project.

- **Regex Entity** - Throughout this project, the term regex entity is used quite a bit in slightly
different contexts. In general, I consider a regex entity to be an data object that contains a regex
and some related metadata. The actual data contained within this entity can vary, but it should always
at least have a regex. Look at some different use cases to determine which data should be available.

- **RE2** - RE2 is a regex engine that we used in this project. It is really fast and has nice time
guarantees.

- **Query** - queries are used to find regexes in our database. Queries are all query by example.
Basically, you provide a set of positive and negative examples, which in turn are used to find
regexes in the database that fit those constraints.

## Source overview
I will try my best to provide an overview of the source tree and what is where. This list may
be somewhat incomplete.

- `librereuse/`
This module provides a library of code that is shared across the different search and evaluation
tools. It contains an actual implementation of the regex search engine, classes for describing
queries, a common representation of regex entities, and other utilities. Code is broken down into
subfolders, so check inside of those for more details.

- `clustering/`
This module contains code for clustering regex entities. There are a few different clustering
implementations contained within. The most important submodules are `cluster-tool` and `filter`.
`filter` provides a program that takes in a list of regex entities and filters out any regexes
that cannot be parsed by RE2 or otherwise cannot be handled by our database. `cluster-tool`
provides a binary to create clusters from all of the regexes that we passed in.

- `dfacover/`
This module provides code for measuring DFA coverage. It uses a fork of brics that allows us
to check how many states and pathes are covered when a string is evaluated on a regex.

- `performance/`
Provides a tool for measuring the performance of the regex search engine. This tool is just used
for measuring the time it takes to complete queries. It does *not* account for how good the results
are or anything like that.

- `regexdb/`
Provides the actual backend web service for the regex search engine. This tool uses gRPC to communicate
with the binary provided in *server*. This is really more of a microservice and shouldn't need to be
expanded much.

- `server/`
Contains all of the server code for api.regextools.io. This server acts as a facade over the *dfacover*
and *regexdb* microservices. Basically, it uses gRPC to communicate with those to services and handle
requests. It also adds additional functionality like sorting results etc.

## Build instructions

To build:
1. Install Bazel build tool ([instructions](https://bazel.build/install/bazelisk)).
2. In the root of this directory, run `$ bazel sync`
3. Run `$ bazel run //regexdb` to run the regex database service
4. Run `$ bazel run //server` to run the API server

I **highly** recommend you use CLion as your primary IDE. It has *excellent* bazel integration and
makes life way easier. You are eligable to use it for free with your student account.

## Some general notes on implementation

As you look through the code, there might be some design choices/decisions that were made that might
seem like bad ideas. In my defence, here are some rationales for some of those decisions.

### *Why C++? I hate C++!*
I would agree that C++ is an interesting language choice. It mainly started because we decided to use
RE2 as our regex engine, which is written natively in C++. That required us to build out the regex
search engine and most of the code interacting with it in that language. Afterall, it is a very effecitve
systems language, so it makes sense to use it.

At some point, a lot of C++ code got written, so it made sense to build additional tools off of the foundation
we already had. So although it may seem like a weird choice to write so much of this tool in such a complicated
language, it made sense at the time considering how much of a foundation the rest of the project had.

### *Why bazel? This thing is waaaaaay too complicated!*
One of the main weaknesses of C++ IMO is it's tooling. Managing dependencies and taking care of dependencies
is super, super annoying. Bazel is a build tool used by a major engineering company designed to make this part
of managing C++ projects easier.

I think the tool has a lot of benefits. Typically, building this project "just works" (although TBH sometimes it
doesn't.) In general dependencies are found easily, and it's easy to build each component with caching and such.
I also think that the build language is a lot eaiser to follow than alternatives like CMake (that tool is my enemy.)

Despite its merits, bazel has some major downsides. The main two are its large disk usage and the coupling between
the build tool and the targets built. First, bazel uses a ton of disk space to facilitate dependency management.
Basically, it usually  likes to clone everything from source and build manually. So, this tool is not ideal for
small disks. Additonally, some of the tools that get built are a little too tied to the build system. Scripting
languages usually need wrappers that can't move around the rest of the file system. As such, building a tool and
"installing" it somewhere else on the computer is tough.

That being said, I think the tool helps in the end. There are other similar tools like buck, buck2 and please, but
bazel had the best IDE tooling and documentation IMO. Buck2 is very promising, but it's very new.

### *What's with all the polymorphism? It seems like some of this stuff isn't used*
I've been working on this codebase for about 3ish years now. During that time, we have had many proposed features
and design ideas that never quite got actualized. As such, there are lots of spaces where I designed code in such
a way that it could be extended for future features. However, some of those future features never actually got
implemented. Now, I am paying for my sins of premature optimization >:\
