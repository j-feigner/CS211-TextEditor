The development of the compression procedures for this milestone went pretty smooth. Unlike many of the previous milestones, I didn't experience many hiccups during the design process that slowed my progress significantly. It was very refreshing to make consisitent, steady progress towards the feature's proof of concept.

My compression is done by word rather than by character.
My program, when commanded to save the current file, will scan the text data being edited for groups of consequitive characters deliminated by non-letter characters. Each of these groups of characters forms a word. 
The frequency of each unique word is determined, and each is then given a unique binary identifier based on their frequency: the most common word begins at 0, the next is 1, then 10, 11 etc.
This process required the use of a priority queue to ensure the most commonly used words in the text received the smallest possible binary string value.

The binary strings are then output to a plain text file, with spaces, punctuation, and newlines (any non-letter character) being preserved in the output for simplicity.
The corresponding word codes are stored in another file (CSV format). This allows for decoding of the compressed text back into a usable form for the editor, however I did not have time to implement this feature. I hope to revisit this in the future.

Although I was successful in creating a compresison procedure for the text, it isn't particularly good at, well, compression.
This is especially true when the text data contains a lot of unique words. The binary strings quickly become larger than the words themselves, and my process only inflates the file size, rather than compressing it.
This problem could perhaps be remedied by doing a more thorough encoding of non-letter characters so the output file could be in binary format, rather than character strings of zeroes, ones, and deliminating characters.

In a text of reasonable size (100+ words), with few unique words, the highest compression ratios I saw was 57%. Not great.

Despite this, I still believe that this milestone was a valuable teaching tool, both with coding and with design.
The coding was challenging, but not overwhelming, and required the use of an new data structure, the priority queue. It was intuituve and easy to implemenet with the operator overload provided (that I tried and failed to implement myself). 
From a design perspective, I learned the importance of actually designing an algorithm and planning its implementation rather than just typing away and seeing what sticks.
Had I been more diligent with design, I perhaps could have made a more robust, or at least more effective, compression procedure.
