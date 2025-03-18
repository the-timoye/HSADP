import java.util.*;
import java.util.concurrent.*;

public class SuffixTreeSearch {

    public static Optional<List<Integer>> createHierarchy(String S, String Q, int l, List<Integer> SA, 
                                                           Optional<Map.Entry<String, List<Integer>>> validIndices, 
                                                           int level) {
        System.out.println("Level: " + level);
        Map<String, List<Integer>> hier = new ConcurrentHashMap<>();
        
        // Determine suffix array for the current level
        List<Integer> suffixArray = validIndices.map(Map.Entry::getValue).orElse(SA);
        
        // Adjust pruning parameter
        int newL = (level > 0) ? ((Q.length() < l + l) ? (l + (Q.length() - l)) : (l + l)) : l;

        // Parallel processing of suffixes using Java Streams
        suffixArray.parallelStream().forEach(index -> {
            if (index + newL > S.length()) return;  // Prevent out-of-bounds errors
            String presentSuffix = S.substring(index, index + newL);
            hier.computeIfAbsent(presentSuffix, k -> Collections.synchronizedList(new ArrayList<>())).add(index);
        });

        // Print tree structure
        System.out.println("Tree Structure: " + hier);

        // Find valid indices for the next recursion
        Optional<Map.Entry<String, List<Integer>>> nextValidIndices = hier.entrySet().stream()
            .filter(entry -> entry.getKey().equals(Q) || entry.getKey().equals(Q.substring(0, newL)))
            .findFirst();

        if (nextValidIndices.isPresent() && nextValidIndices.get().getKey().equals(Q)) {
            return Optional.of(nextValidIndices.get().getValue());  // Found exact match
        }

        if (!nextValidIndices.isPresent()) {
            System.out.println("No matching strings for this Query.");
            return Optional.empty();
        }

        return createHierarchy(S, Q, newL, SA, nextValidIndices, level + 1);
    }

    public static void main(String[] args) {
        String S = "ATGCCTGATGC$";
        String Q = "CCTA";
        List<Integer> SA = Arrays.asList(11, 7, 0, 10, 3, 4, 6, 9, 2, 5, 8, 1);
        int l = 2;  // Pruning parameter

        long startTime = System.nanoTime();
        Optional<List<Integer>> result = createHierarchy(S, Q, l, SA, Optional.empty(), 0);
        long stopTime = System.nanoTime();
        System.out.println("Duration: ");
        System.out.println((stopTime - startTime)/1_000_000_000.0);

        // Call function

        // Print final result
        result.ifPresentOrElse(
            indices -> System.out.println("\nMatching String Found at indices: " + indices),
            () -> System.out.println("\nNo exact match found.")
        );
    }
}
