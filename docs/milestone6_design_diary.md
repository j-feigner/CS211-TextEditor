The development of the compression procedures for this milestone went pretty smooth. Unlike many of the previous milestones, I didn't experience many hiccups during the design process that slowed my progress significantly. It was very refreshing to make consisitent, steady progress towards the feature's proof of concept.

I decided to go with a by-word basis for compression rather than by-character. 
This means that my program will scan the text data being edited for groups of characters that are one or larger (words). 
The frequency of each unique word is determined, and each is then given a unique binary identifier based on their frequency: the most common word begins at 0, the next is 1, then 10, 11 etc.
This process required the use of a priority queue to ensure the most commonly used words in the text received the smallest possible binary string value.

The binary strings are then output to a plain text file, with spaces, punctuation, and newlines being preserved in the output for simplicity.
The corresponding word codes are stored in another file (CSV format). This would allow for decoding of the compressed text back into a usable form for the editor, however I did not have time to implement this feature.

Although I was successful in creating a compresison procedure for the text, it isn't particularly good at compressing the file size.
This is especially true when the text data contains a lot of unique words. The binary strings quickly become larger than the words themselves, which is the opposite of compression.
This problem could perhaps be remedied by doing a more thorough encoding of non-letter characters so the output file could be in binary format, rather than just character strings of zeroes and ones.
When words are common in a text of reasonable size (100+ words), the highest compression ratios I tested was 57%. Not great.

Despite this, I still believe that this milestone was a valuable teaching tool, both with coding and with design.
The coding was challenging, but not overwhelming, and required the use of an new data structure, the priority queue. It was intuituve and easy to implemenet with the operator overload provided (that I tried and failed to implement myself). 
From a design perspective, I learned the importance of actually designing an algorithm and planning its implementation rather than just typing away and seeing what sticks.
Had I been more diligent with design, I perhaps could have made a more robust, or at least more effective, compression procedure.
