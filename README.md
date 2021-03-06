# Peer-to-peer-systems
Peer to peer group based, file sharing system, using socket programming, SHA-1, Multithreading.

## Architecture Overview:
The Following entities will be present in the network :
1. Synchronized trackers(2 tracker system) :
a. Maintain information of clients with their files(shared by client) to assist the
clients for the communication between peers
b. Trackers should be synchronized i.e all the trackers if online should be in sync
with each other
2. Clients:
a. User should create an account and register with tracker
b. Login using the user credentials
c. Create Group and hence will become owner of that group
d. Fetch list of all Groups in server
e. Request to Join Group
f. Leave Group
g. Accept Group join requests (if owner)
h. Share file across group: Share the filename and SHA1 hash of the complete file
as well as piecewise SHA1 with the tracker
i. Fetch list of all sharable files in a Group
j. Download file
i. Retrieve peer information from tracker for the file
ii. Core Part: Download file from multiple peers (different pieces of file from
different peers - piece selection algorithm) simultaneously and all the
files which client downloads will be shareable to other users in the same
group. Ensure file integrity from SHA1 comparison
k. Show downloads
l. Stop sharing file
m. Stop sharing all files(Logout)
n. Whenever client logins, all previously shared files before logout should
automatically be on sharing mode
## Working:
1. At Least one tracker will always be online.
2. Client needs to create an account (userid and password) in order to be part of the
network.
3. Client can create any number of groups(groupid should be different) and hence will
be owner of those groups
4. Client needs to be part of the group from which it wants to download the file
5. Client will send join request to join a group
6. Owner Client Will Accept/Reject the request
7. After joining group ,client can see list of all the shareable files in the group
8. Client can share file in any group (note: file will not get uploaded to tracker but only
the <ip>:<port> of the client for that file)
9. Client can send the download command to tracker with the group name and
filename and tracker will send the details of the group members which are currently
sharing that particular file
10. After fetching the peer info from the tracker, client will communicate with peers about
the portions of the file they contain and hence accordingly decide which part of data
to take from which peer (You need to design your own Piece Selection Algorithm)
11. As soon as a piece of file gets downloaded it should be available for sharing
12. After logout, the client should temporarily stop sharing the currently shared files till
the next login
13. All trackers need to be in sync with each other
