use std::env; // to get arugments passed to the program
use std::thread;
/*
* Print the number of partitions and the size of each partition
* @param vs A vector of vectors
*/
fn print_partition_info(vs: &Vec<Vec<usize>>){
    println!("Number of partitions = {}", vs.len());
    for i in 0..vs.len(){
        println!("\tsize of partition {} = {}", i, vs[i].len());
    }
}

/*
* Create a vector with integers from 0 to num_elements -1
* @param num_elements How many integers to generate
* @return A vector with integers from 0 to (num_elements - 1)
*/
fn generate_data(num_elements: usize) -> Vec<usize>{
    let mut v : Vec<usize> = Vec::new();
    for i in 0..num_elements {
        v.push(i);
    }
    return v;
}

/*
* Partition the data in the vector v into 2 vectors
* @param v Vector of integers
* @return A vector that contains 2 vectors of integers

*/
fn partition_data_in_two(v: &Vec<usize>) -> Vec<Vec<usize>>{
    let partition_size = v.len() / 2;
    // Create a vector that will contain vectors of integers
    let mut xs: Vec<Vec<usize>> = Vec::new();

    // Create the first vector of integers
    let mut x1 : Vec<usize> = Vec::new();
    // Add the first half of the integers in the input vector to x1
    for i in 0..partition_size{
        x1.push(v[i]);
    }
    // Add x1 to the vector that will be returned by this function
    xs.push(x1);

    // Create the second vector of integers
    let mut x2 : Vec<usize> = Vec::new();
    // Add the second half of the integers in the input vector to x2
    for i in partition_size..v.len(){
        x2.push(v[i]);
    }
    // Add x2 to the vector that will be returned by this function
    xs.push(x2);
    // Return the result vector
    xs
}

/*
* Sum up the all the integers in the given vector
* @param v Vector of integers
* @return Sum of integers in v
* Note: this function has the same code as the reduce_data function.
*       But don't change the code of map_data or reduce_data.
*/
fn map_data(v: &Vec<usize>) -> usize{
    let mut sum = 0;
    for i in v{
        sum += i;
    }
    sum
}

/*
* Sum up the all the integers in the given vector
* @param v Vector of integers
* @return Sum of integers in v
*/
fn reduce_data(v: &Vec<usize>) -> usize{
    let mut sum = 0;
    for i in v{
        sum += i;
    }
    sum
}

/*
* A single threaded map-reduce program
*/
fn main() {

    // Use std::env to get arguments passed to the program
    let args: Vec<String> = env::args().collect();
    if args.len() != 3 {
        println!("ERROR: Usage {} num_partitions num_elements", args[0]);
        return;
    }
    let num_partitions : usize = args[1].parse().unwrap();
    let num_elements : usize = args[2].parse().unwrap();
    if num_partitions < 1{
      println!("ERROR: num_partitions must be at least 1");
        return;
    }
    if num_elements < num_partitions{
        println!("ERROR: num_elements cannot be smaller than num_partitions");
        return;
    }

    // Generate data.
    let v = generate_data(num_elements);

    // PARTITION STEP: partition the data into 2 partitions
    let xs = partition_data_in_two(&v);

    // Print info about the partitions
    print_partition_info(&xs);

    let mut intermediate_sums : Vec<usize> = Vec::new();

    // MAP STEP: Process each partition

    // CHANGE CODE START: Don't change any code above this line

    // Change the following code to create 2 threads that run concurrently and each of which uses map_data() function to process one of the two partitions
    let mut _xs1: Vec<usize> = xs[0].clone();
    let mut _xs2: Vec<usize> = xs[1].clone();



    let t_one = thread::spawn(move || map_data(&_xs1));
    let t_two = thread::spawn(move || map_data(&_xs2));

    let res_one = t_one.join().unwrap();
    let res_two = t_two.join().unwrap();

    intermediate_sums.push(res_one);
    intermediate_sums.push(res_two);

    // CHANGE CODE END: Don't change any code below this line until the next CHANGE CODE comment

    // Print the vector with the intermediate sums
    println!("Intermediate sums = {:?}", intermediate_sums);

    // REDUCE STEP: Process the intermediate result to produce the final result
    let sum = reduce_data(&intermediate_sums);
    println!("Sum = {}", sum);

    // CHANGE CODE: Add code that does the following:
    // 1. Calls partition_data to partition the data into equal partitions
    // 2. Calls print_partition_info to print info on the partitions that have been created
    // 3. Creates one thread per partition and uses each thread to concurrently process one partition
    // 4. Collects the intermediate sums from all the threads
    // 5. Prints information about the intermediate sums
    // 5. Calls reduce_data to process the intermediate sums
    // 6. Prints the final sum computed by reduce_data

    //get new array with user input partian amount
    let v2 = partition_data(num_partitions, &v);
    print_partition_info(&v2);
    //create a vector to hold the JoinHandles of each vector
    let mut thread_array: Vec<thread::JoinHandle<_>> = Vec::new();
    //vector to hold each threads data results
    let mut thread_results: Vec<usize> = Vec::new();
    //loop through and fill thread vector with threads
    for i in 0..num_partitions {
        let mut v2_sub: Vec<usize> = v2[i].clone();
        thread_array.push(thread::spawn(move || map_data(&v2_sub)));
        
    }
    //loop through and pull threads out of thread vector re-join them with main thread and add results to result vector
    for i in 0..num_partitions {
        let mut temp = thread_array.pop();
        let mut old_thread: thread::JoinHandle<_> = temp.expect("error");
        thread_results.push(old_thread.join().unwrap());

    }   
    println!("Intermediate sums = {:?}", thread_results);

    let sum = reduce_data(&thread_results);
    println!("Sum = {}", sum);
}

/*
* CHANGE CODE: code this function
* Note: Don't change the signature of this function
*
* Partitions the data into a number of partitions such that
* - the returned partitions contain all elements that are in the input vector
* - if num_elements is a multiple of num_partitions, then all partitions must have equal number of elements
* - if num_elements is not a multiple of num_partitions, some partitions can have one more element than other partitions
*
* @param num_partitions The number of partitions to create
* @param v The data to be partitioned
* @return A vector that contains vectors of integers
* 
*/
fn partition_data(num_partitions: usize, v: &Vec<usize>) -> Vec<Vec<usize>>{

    let mut xs: Vec<Vec<usize>> = Vec::new();
    let mut cur_idx = 0;
    //find out how many partitions need +1 to be added to them
    let non_multiples = v.len() % num_partitions;
    //find the size of the partitions
    let partition_size = v.len()/num_partitions;
    //loop through number of partions required to be one larger than standard size if any
    if non_multiples != 0 {
        for i in 0..non_multiples {
            //create a vector to hold values from v and place partition amount +1 in it
            let mut x: Vec<usize> = Vec::new();
            for i in 0..partition_size+1 {
                x.push(v[cur_idx]);
                cur_idx += 1;
            }
            xs.push(x);
        }
    }
    //loop through remaining partitions of normal size and fill them
    for i in 0..(num_partitions - non_multiples) {
        let mut x: Vec<usize> = Vec::new();
        for i in 0..partition_size {
            x.push(v[cur_idx]);
            cur_idx += 1;
        }
        xs.push(x);
    }

    // Return the result vector
    xs
}
