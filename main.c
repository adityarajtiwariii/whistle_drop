#include <stdio.h>

#define MOD_PIN 4821
#define FILE_NAME "reports.txt"
#define TEMP_NAME "temp.txt"

int id, category, status;
long long code;
int wrong_tries = 0, wrong_pins = 0;

long long read_big() {
    long long n;
    int r, c;
    r = scanf("%lld", &n);
    if (r == EOF) n = 0;
    else if (r != 1) n = -1;
    while ((c = getchar()) != '\n' && c != EOF) ;
    return n;
}

int read_number() {
    long long n = read_big();
    if (n > 1000000 || n < -1) return -1;
    return n;
}

int first_char() {
    int c = getchar();
    while (c == ' ' || c == '\t') c = getchar();
    if (c == EOF) c = '\n';
    return c;
}

void print_category(int c) {
    if (c == 1) printf("Security");
    else if (c == 2) printf("Harassment");
    else if (c == 3) printf("Corruption");
    else if (c == 4) printf("Technical");
    else printf("Other");
}

void print_status(int s) {
    if (s == 1) printf("SUBMITTED");
    else if (s == 2) printf("UNDER_REVIEW");
    else if (s == 3) printf("RESOLVED");
    else printf("DISMISSED");
}

int allowed(int from, int to) {
    return (from == 1 && to == 2) || (from == 2 && (to == 3 || to == 4));
}

int read_header(FILE *f) {
    return fscanf(f, "%d %lld %d %d ", &id, &code, &category, &status) == 4;
}

void skip_line(FILE *f) {
    int c;
    while ((c = fgetc(f)) != '\n' && c != EOF) ;
}

void copy_line(FILE *from, FILE *to) {
    int c;
    while ((c = fgetc(from)) != '\n' && c != EOF) fputc(c, to);
    fputc('\n', to);
}

void write_line(FILE *out, int first) {
    int c = first;
    while (c != '\n' && c != EOF) {
        fputc(c, out);
        c = getchar();
    }
    fputc('\n', out);
}

int code_exists(long long c) {
    FILE *f = fopen(FILE_NAME, "r");
    while (f != NULL && read_header(f)) {
        if (code == c) { fclose(f); return 1; }
        skip_line(f); skip_line(f); skip_line(f);
    }
    if (f != NULL) fclose(f);
    return 0;
}

long long new_code() {
    long long c;
    int i;
    FILE *f = fopen("/dev/urandom", "rb");
    if (f == NULL) return -1;
    do {
        c = 0;
        for (i = 0; i < 10; i++) c = c * 10 + fgetc(f) % 10;
    } while (code_exists(c));
    fclose(f);
    return c;
}

int next_id() {
    int last = 0;
    FILE *f = fopen(FILE_NAME, "r");
    while (f != NULL && read_header(f)) {
        last = id;
        skip_line(f); skip_line(f); skip_line(f);
    }
    if (f != NULL) fclose(f);
    return last + 1;
}

void submit_report() {
    int cat, c, new_id;
    long long my_code;
    FILE *f, *t;
    printf("Category: 1 Security  2 Harassment  3 Corruption  4 Technical  5 Other\nChoose: ");
    cat = read_number();
    if (cat < 1 || cat > 5) { printf("Invalid category.\n"); return; }
    printf("Describe the problem: ");
    c = first_char();
    if (c == '\n') { printf("Description cannot be empty.\n"); return; }
    t = fopen(TEMP_NAME, "w");
    if (t == NULL) { printf("Could not save the report.\n"); return; }
    write_line(t, c);
    printf("Evidence link (press Enter to skip): ");
    c = first_char();
    if (c == '\n') fputs("-\n", t);
    else write_line(t, c);
    fclose(t);
    new_id = next_id();
    my_code = new_code();
    if (my_code < 0) {
        printf("Could not create a safe case code.\n");
        remove(TEMP_NAME);
        return;
    }
    f = fopen(FILE_NAME, "a");
    t = fopen(TEMP_NAME, "r");
    if (f == NULL || t == NULL) {
        printf("Could not save the report.\n");
        if (f != NULL) fclose(f);
        if (t != NULL) fclose(t);
        return;
    }
    fprintf(f, "%d %lld %d 1\n", new_id, my_code, cat);
    copy_line(t, f);
    copy_line(t, f);
    fputs("Report received.\n", f);
    fclose(t);
    fclose(f);
    remove(TEMP_NAME);
    printf("\nReport submitted. SAVE YOUR CASE CODE: %010lld\nIt cannot be recovered.\n", my_code);
}

void track_report() {
    long long want;
    FILE *f;
    if (wrong_tries >= 3) { printf("Too many wrong codes. Restart the program.\n"); return; }
    printf("Enter your case code: ");
    want = read_big();
    f = fopen(FILE_NAME, "r");
    while (f != NULL && read_header(f)) {
        if (code == want) {
            printf("Category: "); print_category(category);
            printf("\nStatus: "); print_status(status);
            printf("\nLatest update: ");
            skip_line(f); skip_line(f);
            copy_line(f, stdout);
            fclose(f);
            return;
        }
        skip_line(f); skip_line(f); skip_line(f);
    }
    if (f != NULL) fclose(f);
    wrong_tries++;
    printf("Invalid case code.\n");
}

void view_reports() {
    int fc, fs, shown = 0;
    FILE *f;
    printf("Filter by category (0 = all, 1-5): ");
    fc = read_number();
    printf("Filter by status (0 = all, 1 Submitted, 2 Under review, 3 Resolved, 4 Dismissed): ");
    fs = read_number();
    if (fc < 0 || fc > 5 || fs < 0 || fs > 4) { printf("Invalid filter.\n"); return; }
    f = fopen(FILE_NAME, "r");
    if (f == NULL) { printf("No reports yet.\n"); return; }
    while (read_header(f)) {
        if ((fc == 0 || fc == category) && (fs == 0 || fs == status)) {
            shown++;
            printf("\nReport #%d | ", id);
            print_category(category);
            printf(" | ");
            print_status(status);
            printf("\n  Description: "); copy_line(f, stdout);
            printf("  Evidence: ");     copy_line(f, stdout);
            printf("  Last update: ");  copy_line(f, stdout);
        } else {
            skip_line(f); skip_line(f); skip_line(f);
        }
    }
    fclose(f);
    if (shown == 0) printf("No matching reports.\n");
}

void update_status() {
    int want_id, new_status, c, found = 0;
    FILE *f, *t;
    printf("Report number: ");
    want_id = read_number();
    printf("New status (2 Under review, 3 Resolved, 4 Dismissed): ");
    new_status = read_number();

    f = fopen(FILE_NAME, "r");
    while (f != NULL && read_header(f)) {
        if (id == want_id) { found = 1; break; }
        skip_line(f); skip_line(f); skip_line(f);
    }
    if (f != NULL) fclose(f);
    if (!found) { printf("No report with that number.\n"); return; }
    if (!allowed(status, new_status)) {
        printf("Not allowed. Only SUBMITTED -> UNDER_REVIEW -> RESOLVED/DISMISSED. Closed cases cannot change.\n");
        return;
    }
    printf("Short update for the reporter: ");
    c = first_char();
    if (c == '\n') { printf("Update message cannot be empty.\n"); return; }

    f = fopen(FILE_NAME, "r");
    t = fopen(TEMP_NAME, "w");
    if (f == NULL || t == NULL) {
        printf("Could not update the report.\n");
        if (f != NULL) fclose(f);
        if (t != NULL) fclose(t);
        return;
    }
    while (read_header(f)) {
        if (id == want_id) {
            fprintf(t, "%d %lld %d %d\n", id, code, category, new_status);
            copy_line(f, t); copy_line(f, t);
            skip_line(f); write_line(t, c);
        } else {
            fprintf(t, "%d %lld %d %d\n", id, code, category, status);
            copy_line(f, t); copy_line(f, t); copy_line(f, t);
        }
    }
    fclose(f);
    fclose(t);
    if (rename(TEMP_NAME, FILE_NAME) != 0) {
        remove(FILE_NAME);
        rename(TEMP_NAME, FILE_NAME);
    }
    printf("Status updated.\n");
}

int login() {
    while (wrong_pins < 3) {
        printf("Moderator PIN: ");
        if (read_number() == MOD_PIN) {
            wrong_pins = 0;
            return 1;
        }
        wrong_pins++;
        printf("Wrong PIN.\n");
    }
    return 0;
}

void moderator_menu() {
    int choice = -1;
    if (!login()) { printf("Access denied.\n"); return; }
    while (choice != 0) {
        printf("\n--- Moderator ---\n1. View reports\n2. Update a report\n0. Logout\nChoose: ");
        choice = read_number();
        if (choice == 1) view_reports();
        else if (choice == 2) update_status();
        else if (choice != 0) printf("Invalid choice.\n");
    }
}

int main() {
    int choice = -1;
    while (choice != 0) {
        printf("\n=== WhistleDrop ===\n1. Submit an anonymous report\n2. Track my report\n3. Moderator login\n0. Exit\nChoose: ");
        choice = read_number();
        if (choice == 1) submit_report();
        else if (choice == 2) track_report();
        else if (choice == 3) moderator_menu();
        else if (choice != 0) printf("Invalid choice.\n");
    }
    return 0;
}
